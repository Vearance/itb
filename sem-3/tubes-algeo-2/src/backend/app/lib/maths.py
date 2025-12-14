import numpy as np

logger = __import__('logging').getLogger("uvicorn.error")

# matrix subtraction -> returns A - B


def matrix_subtract(A, B):
    return np.subtract(A, B)


# matrix multiplication -> returns A * B
def matmul(A, B):
    return np.dot(A, B)


# transpose matrix
def transpose(M):
    return np.transpose(M)


# euclidean norm of a vector v
def vector_norm(v):
    return np.linalg.norm(v)


# vector normalization
def normalize(v):
    norm = vector_norm(v)
    return v / norm


''' 
Versi buku
'''
# NOTE: method gram-schmidt to decompose A into Q and R


def qr_decomposition(A):
    num_rows = len(A)
    num_cols = len(A[0])

    # Q zero matrix; Q = m x n
    Q = [[0.0 for _ in range(num_cols)] for _ in range(num_rows)]
    # R zero matrix (upper triangular); R = n x n
    R = [[0.0 for _ in range(num_cols)] for _ in range(num_cols)]

    # Gram–Schmidt
    for j in range(num_cols):
        # ambil kolom ke-j
        # A = [a1 | a2 | ... | an]
        v = [A[i][j] for i in range(num_rows)]

        # hitung u; u1 = a1; uk+1 = ak+1 - (ak+1 dot e1)e1 - ... - (ak+1 dot e(k))e(k)
        for k in range(j):
            qk = [Q[i][k] for i in range(num_rows)]
            dot = sum(v[i] * qk[i] for i in range(num_rows))
            R[k][j] = dot

            for i in range(num_rows):
                v[i] -= dot * qk[i]

        # norm
        norm = vector_norm(v)
        R[j][j] = norm

        # masukkan e = u / norm(u) ke Q
        for i in range(num_rows):
            Q[i][j] = v[i] / norm  # put e = v / norm(v) into Q

    return Q, R


# NOTE: ini QR iteration
def qr_eigen(A, iterations=50):
    n = len(A)
    Ak = A
    Q_total = [[1 if i == j else 0 for j in range(
        n)] for i in range(n)]  # matrix identitas

    for _ in range(iterations):
        Qk, Rk = qr_decomposition(Ak)
        Ak = matmul(Rk, Qk)
        Q_total = matmul(Q_total, Qk)

    eigenvalues = [Ak[i][i] for i in range(n)]
    eigenvectors = Q_total

    return eigenvalues, eigenvectors


'''
Versi optimized
'''
# NOTE: method householder to decompose A into Q and R


def householder_qr(A):
    A = np.array(A, dtype=float)
    m, n = A.shape

    # initialize Q = I, R = A
    Q = np.eye(m)
    R = np.copy(A)

    for k in range(n):
        # ambil v = R[k:, k]
        x = R[k:, k].copy()

        # hitung norm -> ||x||
        normx = np.linalg.norm(x)
        if normx == 0:
            continue

        # pilih tanda biar stabil
        if (x[0] < 0):
            sign = -1
        else:
            sign = 1

        # hitung Householder vector
        x[0] += sign * normx
        v = x / np.linalg.norm(x)

        # apply reflection ke R: R[k:, k:] -= 2 * outer(v, v @ R[k:, k:])
        R[k:, k:] -= 2 * np.outer(v, np.dot(v, R[k:, k:]))

        # apply reflection ke Q: Q[k:, :] -= 2 * outer(v, v @ Q[k:, :])
        Q[k:, :] -= 2 * np.outer(v, np.dot(v, Q[k:, :]))

    return Q, R


def qr_eigen_shift(A, max_iter=200, tol=1e-10):
    n = len(A)
    Ak = np.array(A, dtype=float)

    # Q_total akan menyimpan eigenvector secara bertahap
    Q_total = np.eye(n)

    for m in range(n, 1, -1):
        for _ in range(max_iter):
            if abs(Ak[m-1, m-2]) < tol * (abs(Ak[m-1, m-1]) + abs(Ak[m-2, m-2]) + 1e-9):
                Ak[m-1, m-2] = 0
                Ak[m-2, m-1] = 0  # Maintain symmetry
                break

            a = Ak[m-2, m-2]
            b = Ak[m-1, m-2]
            c = Ak[m-1, m-1]

            d = (a - c) / 2
            denom = abs(d) + np.sqrt(d*d + b*b)
            if denom == 0:
                mu = c
            else:
                mu = c - (np.copysign(1, d) * b * b) / denom

            shifted = Ak[:m, :m] - mu * np.eye(m)

            Qk, Rk = householder_qr(shifted)

            Ak[:m, :m] = np.dot(Rk, Qk.T) + mu * np.eye(m)

            Q_total[:, :m] = np.dot(Q_total[:, :m], Qk.T)

    eigenvalues = np.diag(Ak)
    eigenvectors = Q_total

    return eigenvalues, eigenvectors


# NOTE: ini eigen-based SVD
def svd(A):
    A = np.array(A, dtype=float)
    logger.debug(f"SVD - Input matrix shape: {A.shape}")

    # B = AT x A  (n x n matrix where n = num_docs)
    B = np.dot(A.T, A)  # num_docs x num_docs
    logger.debug(f"SVD - B = A.T @ A shape: {B.shape}")

    # eigen decomposition of B
    logger.debug("SVD - Starting QR eigen decomposition...")
    eigenvalues, V_eigenvectors = qr_eigen_shift(B)
    logger.debug(f"SVD - Eigenvalues computed: {len(eigenvalues)}")

    # sort dari besar ke kecil by eigenvalue
    order = np.argsort(eigenvalues)[::-1]
    sorted_eigenvalues = eigenvalues[order]
    V = V_eigenvectors[:, order]
    logger.debug(f"SVD - Top 5 eigenvalues: {sorted_eigenvalues[:5]}")

    # singular values: sqrt(eigenvalues) of A^T A
    sing = np.sqrt(np.maximum(sorted_eigenvalues, 0))
    sing[sing < 1e-6] = 0
    logger.debug(f"SVD - Top 5 singular values: {sing[:5]}")

    # compute U = A @ V @ S^{-1}
    # U[:, i] = (1/sigma_i) * A @ v_i
    U = np.zeros((A.shape[0], len(sing)))
    logger.debug(f"SVD - Computing U matrix ({A.shape[0]} x {len(sing)})...")
    for i, sigma in enumerate(sing):
        if sigma < 1e-12:
            continue
        U[:, i] = np.dot(A, V[:, i]) / sigma

    logger.debug(f"SVD - Done! U: {U.shape}, S: {sing.shape}, V: {V.shape}")

    # Transpose V to get VT (for compatibility with LSA code that expects VT)
    VT = V.T
    return U, sing, VT