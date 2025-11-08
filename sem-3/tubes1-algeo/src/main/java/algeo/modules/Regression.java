package algeo.modules;

public class Regression {

    // jadi intinya, variable yang pangkat2 itu diganti jadi variabel baru, misalnya x1, x2, x3, ...

    public static Matrix multiRegression(Matrix X, Matrix y) {
        // B = (X^T . X)^-1  . X^T . y
        Matrix Xt = X.transpose();
        Matrix XtX = MatrixOperator.matrixMultiplication(Xt, X);

        Matrix invM = Inverse.inverseAugment(XtX).matrix;
        Matrix Xty = MatrixOperator.matrixMultiplication(Xt, y);

        Matrix hasil = MatrixOperator.matrixMultiplication(invM, Xty);
        return hasil;
    }

    // ini yang bikin matrixnya
    public static Matrix expandPolynomialMatrix(Matrix Xawal, int derajat) {
        if (derajat <= 1) return Xawal;

        int n = Xawal.getRowsCount();
        int k = Xawal.getColsCount() - 1; // kolom pertama = bias

        // Hitung total suku pake kombinasi
        int totalFeatures = 1; // bias
        for (int d = 1; d <= derajat; d++) {
            totalFeatures += combination(k + d - 1, d);
        }

        double[][] newX = new double[n][totalFeatures];

        for (int i = 0; i < n; i++) {
            int idx = 0;
            newX[i][idx++] = 1.0; // bias

            int[] pangkat = new int[k];
            generatePolynomialMatrix(Xawal, newX, i, k, derajat, 0, pangkat, idxHolder(new int[]{idx}));
        }

        return new Matrix(newX);
    }

    private static void generatePolynomialMatrix(Matrix X, double[][] newX, int row, int k, int maxDeg, int pos, int[] pangkat, int[] idx) {
        if (pos == k) {
            int sum = 0;
            for (int i = 0; i < pangkat.length; i++) {
                sum += pangkat[i];
            }
            if (sum == 0 || sum > maxDeg) return;

            double val = 1.0;
            for (int j = 0; j < k; j++) {
                for (int t = 0; t < pangkat[j]; t++) {
                    val *= X.getElmt(row, j + 1);
                }
            }
            newX[row][idx[0]++] = val;
            return;
        }

        for (int p = 0; p <= maxDeg; p++) {
            pangkat[pos] = p;
            generatePolynomialMatrix(X, newX, row, k, maxDeg, pos + 1, pangkat, idx);
        }
    }

    private static int combination(int n, int r) {
        if (r == 0 || r == n) return 1;
        return combination(n - 1, r - 1) + combination(n - 1, r);
    }

    private static int[] idxHolder(int[] arr) {
        return arr;
    }

    public static String[] generateFeatureNames(int k, int degree) {
        int total = 1;
        for (int d = 1; d <= degree; d++) {
            total += combination(k + d - 1, d);
        }

        String[] names = new String[total];
        int idx = 0;
        names[idx++] = "";

        int[] powers = new int[k];
        idx = generateFeatureNamesRecursive(names, k, degree, 0, powers, idx);
        return names;
    }

    private static int generateFeatureNamesRecursive(String[] names, int k, int maxDeg, int pos, int[] powers, int idx) {
        if (pos == k) {
            int sum = 0;
            for (int p : powers) sum += p;
            if (sum == 0 || sum > maxDeg) return idx;

            // Bangun nama fitur, misal "x1^2x2x3^3"
            StringBuilder sb = new StringBuilder();
            for (int j = 0; j < k; j++) {
                if (powers[j] > 0) {
                    sb.append("x").append(j + 1);
                    if (powers[j] > 1) sb.append("^").append(powers[j]);
                }
            }
            names[idx++] = sb.toString();
            return idx;
        }

        for (int p = 0; p <= maxDeg; p++) {
            powers[pos] = p;
            idx = generateFeatureNamesRecursive(names, k, maxDeg, pos + 1, powers, idx);
        }

        return idx;
    }


    public static double predict(Matrix coefficients, double[] xValues, int degree) {
        int k = xValues.length;

        // Buat vektor X_t (fitur tunggal untuk titik yang mau diprediksi)
        int totalTerms = 1;
        for (int d = 1; d <= degree; d++) {
            totalTerms += combination(k + d - 1, d);
        }

        double[] xt = new double[totalTerms];
        int idx = 0;
        xt[idx++] = 1.0; // bias

        int[] powers = new int[k];
        idx = generateXtRecursive(xValues, xt, k, degree, 0, powers, idx);

        // y_t = X_t * β
        double y = 0.0;
        for (int i = 0; i < coefficients.getRowsCount() && i < xt.length; i++) {
            y += coefficients.getElmt(i, 0) * xt[i];
        }
        return y;
    }

    private static int generateXtRecursive(double[] xValues, double[] xt, int k, int maxDeg, int pos, int[] powers, int idx) {
        if (pos == k) {
            int sum = 0;
            for (int p : powers) sum += p;
            if (sum == 0 || sum > maxDeg) return idx;

            double val = 1.0;
            for (int j = 0; j < k; j++) {
                for (int t = 0; t < powers[j]; t++) {
                    val *= xValues[j];
                }
            }
            xt[idx++] = val;
            return idx;
        }

        for (int p = 0; p <= maxDeg; p++) {
            powers[pos] = p;
            idx = generateXtRecursive(xValues, xt, k, maxDeg, pos + 1, powers, idx);
        }
        return idx;
    }

}
