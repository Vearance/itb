package algeo.modules;

import java.util.Locale;

public class Interpolation {
    /*
     * Melakukan interpolasi polinomial
     * @param points : titik-titik interpolasi
     * @return Matrix object : koefisien dari polinomial
     */
    public static Matrix polynomialInterpolation(Matrix points) {
        int length = points.getRowsCount();
        Matrix augmented = new Matrix(length, length + 1);

        for (int i = 0; i < length; i++) {
            double xi = points.getElmt(i, 0);
            double yi = points.getElmt(i, 1);
            for (int j = 0; j < length; j++) {
                augmented.setElmt(i, j, Math.pow(xi,length - 1 - j));
            }
            augmented.setElmt(i, length, yi);
        }
        return SPL.gaussJordan(augmented).solution;
    }

    /*
     * Melakukan interpolasi splina bezier kubik dengan menggunakan
     * [ 4  1  0 ... 0 ] [ b_1 ]   [ 6*S_1 - S_0   ]
     * [ 1  4  1 ... 0 ] [ b_2 ]   [ 6*S_2         ]
     * [ 0  1  4 ... 0 ] [ b_3 ] = [ 6*S_3         ]
     * [ ...         ..] [ ... ]   [ ...           ]
     * [ 0 ... 0  1  4 ] [b_n-1]   [ 6*S_n-1 - S_n ]
     *
     * @param points : titik-titik interpolasi
     * @return Matrix[] : titik kontrol
     */
    public static Matrix[] interpolasiSplinaBezierKubik(Matrix points) {
        int numPoints = points.getRowsCount();
        if (numPoints < 3) {
            throw new IllegalArgumentException("Dibutuhkan minimal 3 titik untuk interpolasi spline.");
        }
        int systemSize = numPoints - 2;

        Matrix tridiagonal = tridiagonalMatrix(systemSize);
        Matrix xPart = createVectorFromPoints(points, 0);
        Matrix yPart = createVectorFromPoints(points, 1);
        Matrix xSolution = SPL.gauss(Matrix.augment(tridiagonal, xPart)).solution;
        Matrix ySolution = SPL.gauss(Matrix.augment(tridiagonal, yPart)).solution;
        Matrix[] result = new Matrix[xSolution.getRowsCount()];
        for (int i = 0; i < xSolution.getRowsCount(); i++) {
            double mx = xSolution.getElmt(i, 0);
            double my = ySolution.getElmt(i, 0);
            result[i] = new Matrix(new double[][] { { mx, my } });
        }
        return result;
    }

    /*
     * Membuat tridiagonal matrix sesuai ukuran
     * @param size : ukuran matrix tridiagonal
     * @return Matrix object : tridiagonal matrix
     */
    private static Matrix tridiagonalMatrix(int size) {
        Matrix matrix= new Matrix(size, size);

        for (int i = 0; i < size; i++) {
            matrix.setElmt(i, i, 4);

            if (i < size - 1) {
                matrix.setElmt(i, i + 1, 1);
            }

            if (i > 0) {
                matrix.setElmt(i, i - 1, 1);
            }
        }
        return matrix;
    }

    /*
     * Fungsi pembantu untuk membuat vektor konstanta (sisi kanan dari SPL)
     * dari titik-titik kontrol spline, berdasarkan formula spline kubik.
     * @param points Matriks yang berisi titik-titik kontrol utama (S).
     * @param col    Indeks kolom yang akan diproses (0 untuk koordinat x, 1 untuk koordinat y).
     * @return Sebuah matriks kolom (vektor) yang akan digunakan sebagai sisi kanan dari SPL.
     * [ 6*S_1 - S_0   ]
     * [ 6*S_2         ]
     * [ 6*S_3         ]
     * [ ...           ]
     * [ 6*S_n-1 - S_n ]
     */
    private static Matrix createVectorFromPoints(Matrix points, int col) {
        int vectorSize = points.getRowsCount() - 2;

        Matrix resultVector = new Matrix(vectorSize, 1);

        for (int i = 0; i < vectorSize; i++) {
            double val = 6 * points.getElmt(i + 1, col);
            resultVector.setElmt(i, 0, val);
        }

        double firstVal = resultVector.getElmt(0, 0) - points.getElmt(0, col);
        resultVector.setElmt(0, 0, firstVal);

        double lastVal = resultVector.getElmt(vectorSize - 1, 0) - points.getElmt(points.getRowsCount() - 1, col);
        resultVector.setElmt(vectorSize - 1, 0, lastVal);

        return resultVector;
    }
}

