from app.lib.text.utils import (
    preprocess_text,
    compute_tf,
    compute_idf,
    cosine_similarity,
)
from scipy import sparse
import numpy as np
import json
import os
from app.lib.maths import svd, normalize
from app.lib.text.process import TextProcessor

logger = __import__('logging').getLogger("uvicorn.error")


class LSA_Model:
    def __init__(self, mapper_path="data/mapper.json"):
        self.processor = TextProcessor(mapper_path)

        self.docs = []   # token per doc
        self.vocab = []  # unique vocab
        self.tf_list = []  # list of tf
        self.idf = {}  # idf per term
        self.matrix = []  # tf-idf matrix

        # SVD outputs
        self.U = None
        self.S = None
        self.VT = None

        # matrix SVD reduced
        self.VT_k = None

        # (book_id, title) per doc
        self.book_meta = None

    def preprocess_all(self):
        raw_texts = self.processor.get_all_texts()
        logger.debug(f"jumlah dokumen: {len(raw_texts)}")

        self.book_meta = self.processor.get_book_meta()

        for i, text in enumerate(raw_texts):
            logger.debug(f"preprocess doc {i} MULAI")  # TODO: remove debug

            # [2.2.1] tokenize, normalize, stopwords, stemming
            tokens = preprocess_text(text)

            logger.debug(f"preprocess doc {i} SELESAI. tokens={len(tokens)}")

            self.docs.append(tokens)  # put into list

    def build_vocab(self):
        vocab_set = set()  # set -> unique

        for tokens in self.docs:
            vocab_set.update(tokens)

        # sort vocab
        self.vocab = sorted(list(vocab_set))
        logger.debug(f"vocab size: {len(self.vocab)}")

    def compute_all_tf(self):
        self.tf_list = []

        for i, tokens in enumerate(self.docs):
            tf = compute_tf(tokens)
            self.tf_list.append(tf)
            logger.debug(f"TF computed for doc {i}, unique terms: {len(tf)}")

    def compute_idf(self):
        self.idf = compute_idf(self.docs)
        logger.debug(f"IDF computed, terms: {len(self.idf)}")

    def build_matrix(self):
        num_terms = len(self.vocab)
        num_docs = len(self.tf_list)

        term_to_idx = {term: i for i, term in enumerate(self.vocab)}

        rows = []
        cols = []
        data = []

        for doc_idx, tf in enumerate(self.tf_list):
            if doc_idx % 100 == 0:
                logger.debug(f"build_matrix progress: {doc_idx}/{num_docs}")

            for term, tf_val in tf.items():
                if term in term_to_idx:
                    term_idx = term_to_idx[term]
                    idf_val = self.idf.get(term, 0)
                    tfidf = tf_val * idf_val

                    if tfidf != 0:
                        rows.append(term_idx)
                        cols.append(doc_idx)
                        data.append(tfidf)

        sparse_matrix = sparse.coo_matrix(
            (data, (rows, cols)),
            shape=(num_terms, num_docs),
            dtype=np.float32
        )
        self.matrix = sparse_matrix.toarray()

        logger.debug(
            f"TF-IDF matrix built: {self.matrix.shape[0]} x {self.matrix.shape[1]}")

    def compute_svd(self):
        logger.debug("Starting SVD computation...")
        self.U, self.S, self.VT = svd(self.matrix)
        logger.debug(
            f"SVD done. U: {np.array(self.U).shape}, S: {np.array(self.S).shape}, VT: {np.array(self.VT).shape}")

    def reduce_dimension(self, k=50):
        logger.debug(f"Reducing dimension to k={k}")
        self.k = k
        U = np.array(self.U)
        S = np.array(self.S)
        VT = np.array(self.VT)

        # # ambil k kolom pertama dari U (vocab x k)
        # self.U_k = []
        # for row in self.U:
        #     new_row = []
        #     for i in range(k):
        #         new_row.append(row[i])
        #     self.U_k.append(new_row)

        # # ambil k singular values pertama
        # self.S_k = []
        # for i in range(k):
        #     self.S_k.append(self.S[i])

        # # ambil k baris pertama dari VT (k x docs)
        # self.VT_k = []
        # for i in range(k):
        #     old_row = self.VT[i]
        #     new_row = []

        #     # ambil seluruh isi baris (karena yang di-cut hanya row, bukan column)
        #     for val in old_row:
        #         new_row.append(val)

        #     self.VT_k.append(new_row)

        # NOTE: if we can use numpy:
        self.U_k = U[:, :k]
        self.S_k = S[:k]
        self.VT_k = VT[:k, :]
        logger.debug(
            f"Reduced: U_k: {self.U_k.shape}, S_k: {self.S_k.shape}, VT_k: {self.VT_k.shape}")

    def compute_doc_vectors(self, k):
        logger.debug(f"Computing doc vectors for k={k}")
        self.doc_vectors = []
        num_docs = len(self.book_meta)

        for d in range(num_docs):
            vec = [self.S_k[i] * self.VT_k[i][d] for i in range(k)]
            vec = normalize(vec)
            self.doc_vectors.append(vec)
            if d % 100 == 0:
                logger.debug(f"doc_vectors progress: {d}/{num_docs}")

        logger.debug(f"doc_vectors computed: {len(self.doc_vectors)} vectors")

    # def query_to_vector(self, query, k=50):
    #     tokens = preprocess_text(query)

    #     tfq = compute_tf(tokens)

    #     q_vec = []
    #     for term in self.vocab:
    #         q_vec.append(tfq.get(term, 0) * self.idf.get(term, 0))

    #     q_lsa = [0] * k

    #     for i in range(k):
    #         total = 0

    #         # dot product
    #         for j in range(len(q_vec)):
    #             total += q_vec[j] * self.U_k[j][i]

    #         # q_lsa[i] = total * (1 / self.S_k[i]) # TODO: check lagi, i think this one salah
    #         if self.S_k[i] < 1e-12:
    #             q_lsa[i] = 0
    #         else:
    #             q_lsa[i] = total / self.S_k[i]

    #     return q_lsa

    def query_to_vector(self, query, k=50):
        logger.debug(f"query_to_vector: query='{query}', k={k}")
        tokens = preprocess_text(query)
        logger.debug(f"query tokens: {tokens}")
        tfq = compute_tf(tokens)
        logger.debug(f"query TF: {tfq}")

        q_vec = [tfq.get(term, 0) * self.idf.get(term, 0)
                 for term in self.vocab]
        logger.debug(
            f"q_vec non-zero count: {sum(1 for v in q_vec if v != 0)}")

        q_lsa = [0] * k

        for i in range(k):
            total = 0
            for j in range(len(q_vec)):
                total += q_vec[j] * self.U_k[j][i]

            # Project onto LSA space: q_lsa[i] = (q^T @ U_k[:, i])
            # No division by singular values - that amplifies noise!
            q_lsa[i] = total

        result = normalize(q_lsa)
        logger.debug(f"q_lsa norm: {sum(v**2 for v in result)**0.5:.4f}")
        return result

    # def search(self, query, top_k=5, k=50):
    #     # 1. Ubah query menjadi vektor di ruang LSA (ruang 50 dimensi or 'k') -> supaya query dan dokumen berada di field yang sama; same as embed query
    #     q_lsa = self.query_to_vector(query, k)

    #     # 2. Buat vektor dokumen dalam ruang LSA; Setiap dokumen punya 50 angka yang menggambarkan "makna leksikal" dokumen; same as embed document
    #     doc_vectors = []
    #     num_docs = len(self.book_meta)

    #     for d in range(num_docs):
    #         vec = []
    #         for i in range(k):
    #             # Nilai dokumen pada dimensi ke-i = (singular value) x (komponen VT)
    #             vec.append(self.S_k[i] * self.VT_k[i][d])
    #         vec = normalize(vec)
    #         doc_vectors.append(vec)

    #     # 3. hitung seberapa mirip query vs setiap dokumen; similarity search
    #     scores = []
    #     for idx, doc_vec in enumerate(doc_vectors):
    #         score = cosine_similarity(q_lsa, doc_vec)
    #         scores.append((idx, score))

    #     # 4. urutkan dokumen berdasarkan skor kemiripan (paling mirip dulu)
    #     scores.sort(key=lambda x: x[1], reverse=True)

    #     # 5. ambil top-k dokumen yang paling mirip dan isi dengan metadata buku
    #     results = []
    #     for idx, score in scores[:top_k]:
    #         book_id, title = self.book_meta[idx]
    #         results.append({
    #             "id": book_id,
    #             "title": title,
    #             "score": score
    #         })

    #     return results

    def search(self, query, top_k=5, k=50):
        logger.debug(f"search: query='{query}', top_k={top_k}, k={k}")
        q_lsa = self.query_to_vector(query, k)

        doc_vectors = self.doc_vectors   # precomputed!

        scores = []
        for idx, doc_vec in enumerate(doc_vectors):
            score = cosine_similarity(q_lsa, doc_vec)
            scores.append((idx, score))

        scores.sort(key=lambda x: x[1], reverse=True)
        logger.debug(f"top 5 scores: {scores[:5]}")

        results = []
        for idx, score in scores[:top_k]:
            book_id, title = self.book_meta[idx]
            results.append(
                {"id": book_id, "title": title, "score": float(score)})

        return results

    def recommend_by_id(self, book_id, top_k=5):
        # Find index of the book
        target_idx = None
        for idx, (bid, title) in enumerate(self.book_meta):
            if str(bid) == str(book_id):
                target_idx = idx
                break

        if target_idx is None:
            print(f"[ERROR] Book ID {book_id} not found!")
            return []

        target_title = self.book_meta[target_idx][1]
        print(
            f"[INFO] Finding recommendations for: {target_title} (ID: {book_id})")

        # Get target document vector
        target_vec = self.doc_vectors[target_idx]

        # Compute similarity with all other documents
        scores = []
        for idx, doc_vec in enumerate(self.doc_vectors):
            if idx == target_idx:
                continue  # Skip the same book
            score = cosine_similarity(target_vec, doc_vec)
            scores.append((idx, score))

        # Sort by similarity
        scores.sort(key=lambda x: x[1], reverse=True)

        # Return top-k
        results = []
        for idx, score in scores[:top_k]:
            bid, title = self.book_meta[idx]
            results.append({
                "id": bid,
                "title": title,
                "score": float(score)
            })

        return results


def load_lsa_from_cache(cache_dir="cache", k=50):
    logger.info("Loading cached data...")

    lsa = LSA_Model.__new__(LSA_Model)

    # Load vocabulary
    with open(os.path.join(cache_dir, "vocab.json"), "r", encoding="utf-8") as f:
        lsa.vocab = json.load(f)
    logger.info(f"vocab.json: {len(lsa.vocab)} terms")

    # Load TF list
    with open(os.path.join(cache_dir, "tf_list.json"), "r", encoding="utf-8") as f:
        lsa.tf_list = json.load(f)
    logger.info(f"tf_list.json: {len(lsa.tf_list)} documents")

    # Load IDF
    with open(os.path.join(cache_dir, "idf.json"), "r", encoding="utf-8") as f:
        lsa.idf = json.load(f)
    logger.info(f"idf.json: {len(lsa.idf)} terms")

    # Load book metadata
    with open(os.path.join(cache_dir, "book_meta.json"), "r", encoding="utf-8") as f:
        lsa.book_meta = json.load(f)
    logger.info(f"book_meta.json: {len(lsa.book_meta)} books")

    # Load TF-IDF matrix (sparse -> dense numpy array)
    tfidf_sparse = sparse.load_npz(os.path.join(cache_dir, "tfidf_matrix.npz"))

    lsa.matrix = tfidf_sparse.toarray()
    logger.info(f"tfidf_matrix.npz: {tfidf_sparse.shape}")

    lsa.doc_vectors = np.load(os.path.join(cache_dir, "doc_vectors.npy"))
    logger.info(f"doc_vectors.npy: {len(lsa.doc_vectors)} vectors")

    # Load U_k and S_k
    U_k_sparse = sparse.load_npz(os.path.join(cache_dir, "u_k.npz"))
    lsa.U_k = U_k_sparse.toarray()
    lsa.S_k = np.load(os.path.join(cache_dir, "s_k.npy"))
    logger.info(f"U_k loaded: {lsa.U_k.shape}, S_k loaded: {lsa.S_k.shape}")

    return lsa


def build_and_cache_lsa(cache_dir="cache", k=50, mapper_path="data/mapper.json"):
    lsa = LSA_Model(mapper_path=mapper_path)
    lsa.preprocess_all()
    lsa.build_vocab()
    lsa.compute_all_tf()
    lsa.compute_idf()
    lsa.build_matrix()
    lsa.compute_svd()
    lsa.reduce_dimension(k)
    lsa.compute_doc_vectors(k)

    logger.info("Caching data...")

    # Save vocabulary
    with open(os.path.join(cache_dir, "vocab.json"), "w", encoding="utf-8") as f:
        json.dump(lsa.vocab, f)
    logger.info(f"vocab.json saved: {len(lsa.vocab)} terms")

    # Save TF list
    with open(os.path.join(cache_dir, "tf_list.json"), "w", encoding="utf-8") as f:
        json.dump(lsa.tf_list, f)
    logger.info(f"tf_list.json saved: {len(lsa.tf_list)} documents")

    # Save IDF
    with open(os.path.join(cache_dir, "idf.json"), "w", encoding="utf-8") as f:
        json.dump(lsa.idf, f)
    logger.info(f"idf.json saved: {len(lsa.idf)} terms")

    # Save book metadata
    with open(os.path.join(cache_dir, "book_meta.json"), "w", encoding="utf-8") as f:
        json.dump(lsa.book_meta, f)
    logger.info(f"book_meta.json saved: {len(lsa.book_meta)} books")

    # Save TF-IDF matrix (sparse)
    tfidf_sparse = sparse.coo_matrix(lsa.matrix)
    sparse.save_npz(os.path.join(cache_dir, "tfidf_matrix.npz"), tfidf_sparse)
    logger.info(f"tfidf_matrix.npz saved: {tfidf_sparse.shape}")

    np.save(os.path.join(cache_dir, "doc_vectors.npy"), lsa.doc_vectors)
    logger.info(f"doc_vectors.npy saved: {len(lsa.doc_vectors)} vectors")

    sparse.save_npz(os.path.join(cache_dir, "u_k.npz"),
                    sparse.coo_matrix(lsa.U_k))
    np.save(os.path.join(cache_dir, "s_k.npy"), lsa.S_k)
    logger.info(f"U_k saved: {lsa.U_k.shape}, S_k saved: {lsa.S_k.shape}")

    return lsa
