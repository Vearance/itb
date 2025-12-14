import nltk
import re
import math
from collections import Counter
from nltk.tokenize import word_tokenize
# from nltk.corpus import stopwords
from nltk.stem import PorterStemmer
import numpy as np

nltk.download("punkt", quiet=True)
nltk.download("punkt_tab", quiet=True)
# nltk.download("stopwords", quiet=True)

# STOPWORDS = set(stopwords.words("english"))
STOPWORDS = set(["a", "about", "above", "after", "again", "against", "ain", "all", "am", "an", "and", "any", "are", "aren", "aren't", "as", "at", "be", "because", "been", "before", "being", "below", "between", "both", "but", "by", "can", "couldn", "couldn't", "d", "did", "didn", "didn't", "do", "does", "doesn", "doesn't", "doing", "don", "don't", "down", "during", "each", "few", "for", "from", "further", "had", "hadn", "hadn't", "has", "hasn", "hasn't", "have", "haven", "haven't", "having", "he", "her", "here", "hers", "herself", "him", "himself", "his", "how", "i", "if", "in", "into", "is", "isn", "isn't", "it", "it's", "its", "itself", "just", "ll", "m", "ma", "me", "mightn", "mightn't", "more", "most", "mustn", "mustn't", "my", "myself", "needn", "needn't", "no", "nor", "not", "now", "o", "of", "off", "on", "once", "only", "or", "other", "our", "ours", "ourselves", "out", "over", "own", "re", "s", "same", "shan", "shan't", "she", "she's", "should", "should've", "shouldn", "shouldn't", "so", "some", "such", "t", "than", "that", "that'll", "the", "their", "theirs", "them", "themselves", "then", "there", "these", "they", "this", "those", "through", "to", "too", "under", "until", "up", "ve", "very", "was", "wasn", "wasn't", "we", "were", "weren", "weren't", "what", "when", "where", "which", "while", "who", "whom", "why", "will", "with", "won", "won't", "wouldn", "wouldn't", "y", "you", "you'd", "you'll", "you're", "you've", "your", "yours", "yourself", "yourselves", "could", "he'd", "he'll", "he's", "here's", "how's", "i'd", "i'll", "i'm", "i've", "let's", "ought", "she'd", "she'll", "that's", "there's", "they'd", "they'll", "they're", "they've", "we'd", "we'll", "we're", "we've", "what's", "when's", "where's", "who's", "why's", "would"])
STEMMER = PorterStemmer()

# Cache untuk memoize hasil stemming
stem_cache = {}

def cached_stem(word):
    if word not in stem_cache:
        stem_cache[word] = STEMMER.stem(word)
    return stem_cache[word]

def clean_text(text: str) -> str:
    text = re.sub(r"[^\w\s]", " ", text)
    text = re.sub(r"\s+", " ", text)
    return text.strip()

def preprocess_text(text: str):
    # Clean and lowercase in one go
    text = clean_text(text).lower()
    
    # Tokenize directly (skip sentence splitting)
    tokens = word_tokenize(text)
    
    # Filter and stem
    processed = []
    for token in tokens:
        if token.isalpha() and token not in STOPWORDS:
            processed.append(cached_stem(token))
    
    return processed

# term frequency -> how often a term appears in a document
def compute_tf(tokens):
    total = len(tokens)
    count = Counter(tokens)
    return {term: count[term] / total for term in count}

# inverse document frequency -> how common/rare a term is across all documents; if its common, it gets lower weight
def compute_idf(docs):
    N = len(docs)
    df = Counter()
    for tokens in docs:
        for term in set(tokens):
            df[term] += 1

    idf = {}
    for term, count in df.items():
        idf[term] = math.log10(N / (1 + count))
    return idf

def cosine_similarity(v1, v2):
    return np.dot(v1, v2) / (np.linalg.norm(v1) * np.linalg.norm(v2))
