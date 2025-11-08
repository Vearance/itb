package algeo.modules;

public class Determinant {
    private static final int DIMENSION_THRESHOLD = 11;
    /*
     * Menghitung determinan matriks dengan metode ekspansi kofaktor.
     * Behaviors :
     * 1. dimensi matrix <= DIMENSION_THRESHOLD, langkah-langkah akan dicatat.
     * 2. dimensi matrix > DIMENSION_THRESHOLD, langkah-langkah tidak akan dicatat.
     *
     * @param a Matriks persegi yang akan dihitung determinannya.
     * @return DeterminantResult object yang berisi nilai determinan dan langkah-langkahnya.
     */
    public static DeterminantResult detCofactor (Matrix a) {
        if (!a.isSquare()) {
            throw new IllegalStateException("Determinan hanya bisa dihitung dari matriks persegi.");
        }

        if (a.getRowsCount() > DIMENSION_THRESHOLD) {
            double result = detCofactorHelper(a, 0, null);
            String stepString = "Langkah-langkah tidak ditampilkan karena dimensi matrix > " + DIMENSION_THRESHOLD + " x " + DIMENSION_THRESHOLD;
            return new DeterminantResult(result, stepString);
        }

        StringBuilder steps = new StringBuilder("Menghitung determinan dengan Metode Ekspansi Kofaktor:\n\n");
        steps.append("Matriks Awal:\n").append(a).append("\n");

        double result = detCofactorHelper(a, 0, steps);

        steps.append("\n========================================\n");
        steps.append("Total Determinan Akhir = ").append(String.format("%.3f", result)).append("\n");

        return new DeterminantResult(result, steps.toString());
    }

    /*
     * Fungsi rekursif pembantu untuk metode ekspansi kofaktor.
     * Menghitung determinan sub-matriks dan secara opsional mencatat langkah.
     *
     * @param a     Matriks atau sub-matriks yang sedang diproses dalam langkah rekursi.
     * @param depth Tingkat kedalaman rekursi, digunakan untuk mengatur indentasi pada teks langkah.
     * @param steps StringBuilder untuk menampung string langkah. Bernilai null jika langkah tidak dicatat.
     * @return nilai determinan dari matriks input (bertipe double).
     */
    private static double detCofactorHelper(Matrix a, int depth, StringBuilder steps) {
        String indentation = "  ".repeat(depth);
        int n = a.getRowsCount();

        if (steps != null) {
            steps.append(indentation).append("-> Menghitung determinan matriks ").append(n).append("x").append(n).append("\n");
        }

        if (n == 1) {
            double val = a.getElmt(0, 0);
            if (steps != null) {
                steps.append(indentation).append("   Hasil = ").append(String.format("%.3f", val)).append("\n");
            }
            return val;
        }

        if (n == 2) {
            double val = a.getElmt(0, 0) * a.getElmt(1, 1) - a.getElmt(0, 1) * a.getElmt(1, 0);
            if (steps != null) {
                steps.append(indentation).append(String.format("   det = (%.3f * %.3f) - (%.3f * %.3f) = %.3f\n",
                        a.getElmt(0, 0), a.getElmt(1, 1), a.getElmt(0, 1), a.getElmt(1, 0), val));
            }
            return val;
        }

        double det = 0.0;
        if (steps != null) {
            StringBuilder formula = new StringBuilder(indentation + "   det = ");
            for(int j = 0; j < n; j++){
                formula.append(String.format("%s (%.3f * C%d%d) ", (j > 0 ? "+ " : ""), a.getElmt(0,j), 1, j+1));
            }
            steps.append(formula.toString()).append("\n");
        }

        for (int j = 0; j < n; j++) {
            Matrix minor = a.removeRowColMatrix(0, j);
            double minorDet = detCofactorHelper(minor, depth + 1, steps);
            det += Math.pow(-1, j) * a.getElmt(0, j) * minorDet;
        }

        if (steps != null) {
            steps.append(indentation).append("   Hasil determinan level ini = ").append(String.format("%.3f", det)).append("\n");
        }
        return det;
    }

    /*
     * Menghitung determinan matriks dengan metode reduksi baris.
     * Behaviors :
     * 1. dimensi matrix <= DIMENSION_THRESHOLD, langkah-langkah OBE akan dicatat.
     * 2. dimensi matrix > DIMENSION_THRESHOLD, langkah-langkah tidak akan dicatat.
     *
     * @param a Matriks persegi yang akan dihitung determinannya.
     * @return DeterminantResult object yang berisi nilai determinan dan langkah-langkahnya.
     */
    public static DeterminantResult detReduksiBaris(Matrix a) {
        if (!a.isSquare()) {
            throw new IllegalStateException("Determinan hanya bisa dihitung dari matriks persegi.");
        }
        if (a.getRowsCount() > DIMENSION_THRESHOLD) {
            double result = detReduksiBarisWithoutSteps(a);
            String stepString = "Langkah-langkah tidak ditampilkan karena dimensi matrix > " + DIMENSION_THRESHOLD + " x " + DIMENSION_THRESHOLD;
            return new DeterminantResult(result, stepString);
        }
        StringBuilder steps = new StringBuilder();
        Matrix m = a.copyMatrix();
        int n = m.getRowsCount();
        int swapCount = 0;
        steps.append("Matriks Awal:\n").append(m).append("\n\n");
        for (int i = 0; i < n; i++) {
            int pivotRow = i;
            while (pivotRow < n && m.getElmt(pivotRow, i) == 0) { pivotRow++; }
            if (pivotRow == n) {
                steps.append("Kolom ").append(i + 1).append(" tidak memiliki pivot. Determinan adalah 0.\n");
                return new DeterminantResult(0, steps.toString());
            }
            if (pivotRow != i) {
                steps.append("-> Tukar Baris ").append(i + 1).append(" dengan Baris ").append(pivotRow + 1).append(".\n");
                m.swapRow(pivotRow, i);
                swapCount++;
            }
            double pivotVal = m.getElmt(i, i);
            for (int j = i + 1; j < n; j++) {
                if (m.getElmt(j, i) != 0) {
                    double factor = m.getElmt(j, i) / pivotVal;
                    steps.append(String.format("-> B%d = B%d - (%.3f * B%d)\n", j + 1, j + 1, factor, i + 1));
                    m.addRowMultiple(j, i, -factor);
                }
            }
            if (i < n - 1) { steps.append("Matriks setelah eliminasi kolom ").append(i + 1).append(":\n").append(m).append("\n\n"); }
        }
        steps.append("========================================\n");
        steps.append("Matriks akhir (bentuk segitiga atas):\n").append(m).append("\n\n");
        double determinant = 1.0;
        StringBuilder calculation = new StringBuilder();
        for (int i = 0; i < n; i++) {
            double diagonalElmt = m.getElmt(i, i);
            determinant *= diagonalElmt;
            calculation.append(String.format("%.3f", diagonalElmt)).append(i < n - 1 ? " * " : "");
        }
        steps.append("Determinan = perkalian elemen diagonal\n");
        steps.append("= ").append(calculation).append(" = ").append(String.format("%.3f", determinant)).append("\n");
        if (swapCount > 0 && swapCount % 2 == 1) {
            determinant *= -1;
            steps.append("Karena jumlah pertukaran baris ganjil (").append(swapCount).append("), hasil dikali -1.\n");
            steps.append("Determinan akhir = ").append(String.format("%.3f", determinant)).append("\n");
        }
        return new DeterminantResult(determinant, steps.toString());
    }

    /*
     * Fungsi pembantu untuk metode reduksi baris tanpa pencatatan langkah.
     * Digunakan untuk efisiensi pada matriks berdimensi besar.
     *
     * @param a Matriks persegi yang akan dihitung determinannya.
     * @return nilai determinan dari matriks input (bertipe double).
     */
    private static double detReduksiBarisWithoutSteps(Matrix a) {
        if (!a.isSquare()) {
            throw new IllegalStateException("Determinan hanya bisa dihitung dari matriks persegi.");
        }
        Matrix m = a.copyMatrix();
        int n = m.getRowsCount();
        double determinant = 1.0;
        int swapCount = 0;
        for (int i = 0; i < n; i++) {
            int pivotRow = i;
            while (pivotRow < n && m.getElmt(pivotRow, i) == 0) {
                pivotRow++;
            }
            if (pivotRow == n) {
                return 0;
            }
            if (pivotRow != i) {
                m.swapRow(pivotRow, i);
                swapCount++;
            }
            double pivotVal = m.getElmt(i, i);
            determinant *= pivotVal;
            for (int j = i + 1; j < n; j++) {
                double factor = m.getElmt(j, i) / pivotVal;
                m.addRowMultiple(j, i, -factor);
            }
        }
        if (swapCount % 2 == 1) {
            determinant *= -1;
        }
        return determinant;
    }
}