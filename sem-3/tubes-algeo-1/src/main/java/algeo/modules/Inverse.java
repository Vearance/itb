package algeo.modules;

public class Inverse {
    private static final int DIMENSION_THRESHOLD  = 11;

    /*
     * Menghitung invers matriks dengan metode matriks augmented [A | I].
     * Metode ini menggunakan eliminasi Gauss-Jordan untuk mengubah [A | I] menjadi [I | A⁻¹].
     * Behaviors :
     * 1. dimensi matrix <= DIMENSION_THRESHOLD, langkah-langkah OBE akan dicatat.
     * 2. dimensi matrix > DIMENSION_THRESHOLD, langkah-langkah tidak akan dicatat.
     *
     * @param a Matriks persegi dan non-singular (determinan != 0) yang akan diinvers.
     * @return InverseResult object yang berisi matriks invers dan langkah-langkahnya.
     */
    public static InverseResult inverseAugment(Matrix a) {
        // dengan [A | I]
        if (!a.isSquare()) {
            throw new IllegalArgumentException("Matriks harus persegi");
        }

        DeterminantResult determinant = Determinant.detReduksiBaris(a);
        if (determinant.value== 0) {
            throw new IllegalArgumentException("Matrix Singular, Invers tidak terdefinisi");
        }


        if (a.getRowsCount() > DIMENSION_THRESHOLD) {
            Matrix inverse = inverseAugmentWithoutSteps(a);
            String stepString = "Langkah-langkah tidak ditampilkan karena dimensi matrix > " + DIMENSION_THRESHOLD + " x " + DIMENSION_THRESHOLD;
            return new InverseResult(inverse, stepString);
        }
        int n = a.getRowsCount();

        // buat matriks [A | I]
        StringBuilder steps = new StringBuilder("Menghitung invers menggunakan metode Augment.\n\n");
        Matrix I = Matrix.identity(n);
        Matrix augmented = Matrix.augment(a.copyMatrix(), I);
        steps.append("Langkah 1: Bentuk matriks augmented [A | I]:\n").append(augmented).append("\n\n");
        steps.append("Langkah 2: Lakukan OBE untuk mengubah matriks menjadi [I | A⁻¹]:\n");
        SPLResult rrefResult = SPL.reducedEchelonFormWithSteps(augmented);
        steps.append(rrefResult.steps);
        Matrix reduced = rrefResult.solution;

        Matrix inverse = new Matrix(n, n);
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                inverse.setElmt(i, j, reduced.getElmt(i, j + n));
            }
        }
        steps.append("\nLangkah 3: Ekstrak bagian kanan matriks untuk mendapatkan A⁻¹:\n").append(inverse);
        return new InverseResult(inverse, steps.toString());
    }

    /*
     * Fungsi pembantu untuk metode matriks augmented tanpa pencatatan langkah.
     * Digunakan untuk efisiensi pada matriks berdimensi besar.
     *
     * @param a Matriks persegi dan non-singular yang akan diinvers.
     * @return matriks invers dari matriks input.
     */
    private static Matrix inverseAugmentWithoutSteps(Matrix a) {
        int n = a.getRowsCount();

        // buat matriks [A | I]
        Matrix I = Matrix.identity(n);
        Matrix augmented = Matrix.augment(a.copyMatrix(), I);

        // pake Gauss Jordan, jadi tereduksi, ini hasilnya [I | A-1]
        Matrix reduced = SPL.reducedEchelonFormWithoutSteps(augmented);
        // salin bagian kanan, atau A-1
        Matrix inverse = new Matrix(n, n);
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                inverse.setElmt(i, j, reduced.getElmt(i, j + n));
            }
        }
        return inverse;
    }

    /*
     * Menghitung invers matriks dengan metode Adjoin.
     * Menggunakan formula: A⁻¹ = (1/det(A)) * Adj(A), di mana Adj(A) adalah transpose dari matriks kofaktor.
     * Behaviors :
     * 1. dimensi matrix <= DIMENSION_THRESHOLD, langkah-langkah akan dicatat.
     * 2. dimensi matrix > DIMENSION_THRESHOLD, langkah-langkah tidak akan dicatat.
     *
     * @param a Matriks persegi dan non-singular (determinan != 0) yang akan diinvers.
     * @return InverseResult object yang berisi matriks invers dan langkah-langkahnya.
     */
    public static InverseResult inverseAdjoin(Matrix a) {
        if (!a.isSquare()) {
            throw new IllegalArgumentException("Invers tidak terdefinisi untuk matriks nonpersegi");
        }

        DeterminantResult determinant = Determinant.detReduksiBaris(a);
        if (determinant.value == 0) {
            throw new IllegalArgumentException("Matrix Singular, Invers tidak terdefinisi");
        }

        if (a.getRowsCount() > DIMENSION_THRESHOLD) {
            Matrix cof = cofactorMatrix(a).matrix;
            Matrix adj = cof.transpose();
            Matrix inv = MatrixOperator.scalarDivision(adj, determinant.value);
            return new InverseResult(inv, "Langkah-langkah tidak ditampilkan untuk Invers Adjoin pada matriks besar.");
        }

        StringBuilder steps = new StringBuilder("Menghitung invers metode Adjoin: A⁻¹ = (1/det(A)) * Adj(A)\n\n");
        steps.append("LANGKAH 1: Menghitung determinan matriks A...\n");
        steps.append("Hasil: det(A) = ").append(String.format("%.3f", determinant.value)).append("\n\n");

        steps.append("LANGKAH 2: Membentuk matriks Adjoin (Adj(A) = Cᵀ).\n");
        CofactorResult cofactorRes = cofactorMatrix(a);
        steps.append(cofactorRes.steps);
        Matrix adjoinMatrix = cofactorRes.matrix.transpose();
        steps.append("Matriks Adjoin (Adj(A)) adalah transpose dari matriks kofaktor:\n").append(adjoinMatrix).append("\n\n");

        steps.append("LANGKAH 3: Hitung invers akhir A⁻¹ = (1/det(A)) * Adj(A).\n");
        Matrix inverse = MatrixOperator.scalarDivision(adjoinMatrix, determinant.value);
        steps.append("Matriks Invers (A⁻¹) hasil akhir:\n").append(inverse);

        return new InverseResult(inverse, steps.toString());
    }

    /*
     * Membentuk matriks kofaktor dari sebuah matriks input.
     * Setiap elemen C(i,j) dari matriks kofaktor adalah (-1)^(i+j) dikali determinan dari minor M(i,j).
     * Behaviors :
     * 1. dimensi matrix <= DIMENSION_THRESHOLD, langkah-langkah akan dicatat.
     * 2. dimensi matrix > DIMENSION_THRESHOLD, langkah-langkah tidak akan dicatat.
     *
     * @param a Matriks persegi yang akan dibuatkan matriks kofaktornya.
     * @return CofactorResult object yang berisi matriks kofaktor dan langkah-langkah pembentukannya.
     */
    public static CofactorResult cofactorMatrix(Matrix a){
        if (!a.isSquare()){
            throw new IllegalStateException("Matriks kofaktor hanya bisa dibentuk dari matriks persegi");
        }

        int n = a.getRowsCount();
        if (n > DIMENSION_THRESHOLD) {
            Matrix result = new Matrix(n,n);
            for (int i = 0; i < n;i++){
                for (int j = 0; j < a.getColsCount(); j++){
                    Matrix r = a.removeRowColMatrix(i,j);
                    double cofactor = Math.pow(-1 , (i + j)) * Determinant.detReduksiBaris(r).value;
                    result.setElmt(i,j,cofactor);
                }
            }
            return new CofactorResult(result, "Langkah-langkah pembentukan matriks kofaktor tidak ditampilkan untuk matriks besar.");
        }

        StringBuilder steps = new StringBuilder("Membentuk Matriks Kofaktor:\n");
        Matrix result = new Matrix(n,n);
        for (int i = 0; i < n;i++){
            for (int j = 0; j < a.getColsCount(); j++){
                steps.append("- Menghitung Kofaktor C(").append(i + 1).append(",").append(j + 1).append(")\n");
                Matrix r = a.removeRowColMatrix(i,j);
                double minorDetValue = Determinant.detReduksiBaris(r).value;
                double cofactor = Math.pow(-1 , (i + j)) * minorDetValue;
                steps.append(String.format("  -> det(Minor) = %.3f -> Kofaktor = %.3f\n", minorDetValue, cofactor));
                result.setElmt(i,j,cofactor);
            }
        }
        steps.append("\nMatriks Kofaktor akhir yang terbentuk:\n").append(result).append("\n");
        return new CofactorResult(result, steps.toString());
    }
}
