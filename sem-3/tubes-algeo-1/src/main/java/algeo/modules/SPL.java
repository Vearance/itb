package algeo.modules;

public class SPL {
    private static final int DIMENSION_THRESHOLD  = 11;

    /*
     * Menyelesaikan SPL menggunakan metode gauss
     * Behaviors :
     * 1. dimensi matrix <= DIMENSION_THRESHOLD, langkah-langkah akan dimunculkan
     * 2. dimensi matrix > DIMENSION_THRESHOLD, langkah-langkah tidak dimunculkan
     *
     * @param augmented
     * @return SPLResult object yang terdiri dari solusi dan langkah-langkah
     */
    public static SPLResult gauss(Matrix augmented) {
        if (augmented.getRowsCount() > DIMENSION_THRESHOLD) {
            Matrix solution = gaussWithoutSteps(augmented);
            return new SPLResult(solution, "Langkah-langkah tidak ditampilkan pada matriks besar.");
        }

        StringBuilder steps = new StringBuilder("Menyelesaikan SPL dengan Eliminasi Gauss:\n\n");
        steps.append("LANGKAH 1: Mengubah matriks ke bentuk Eselon Baris (Row Echelon Form).\n");
        SPLResult echelonResult = echelonFormWithSteps(augmented);
        steps.append(echelonResult.steps);
        Matrix m = echelonResult.solution;

        steps.append("\nLANGKAH 2: Menganalisis hasil matriks eselon baris dan melakukan substitusi balik.\n");

        Matrix a = m.removeLastCol();
        double[] b = m.getCol(m.getColsCount() - 1);

        if (checkNoSolution(a, b)) {
            steps.append("-> Ditemukan baris [0 0 ... | c] dengan c != 0. SPL tidak memiliki solusi.\n");
            return new SPLResult(null, steps.toString());
        } else if (checkUniqueSolution(a, b)) {
            steps.append("-> Setiap variabel adalah variabel pivot. SPL memiliki solusi unik.\n");
            Matrix solution = backSubstitute(a, b);
            steps.append("Solusi unik yang ditemukan:\n").append(solution);
            return new SPLResult(solution, steps.toString());
        } else {
            steps.append("-> Terdapat variabel bebas. SPL memiliki banyak solusi (parametrik).\n");

            // Langsung gunakan hasil dari bentuk eselon baris (REF)
            Matrix refA = m.removeLastCol();
            double[] refB = m.getCol(m.getColsCount() - 1);

            Matrix solution = getParametricSolution(refA, refB);
            steps.append("Solusi Parametrik (kolom pertama adalah konstanta, kolom berikutnya adalah parameter):\n").append(solution);
            return new SPLResult(solution, steps.toString());
        }

    }

    /*
     * Menyelesaikan SPL menggunakan metode gaussJordan
     * Behaviors :
     * 1. dimensi matrix <= DIMENSION_THRESHOLD, langkah-langkah akan dimunculkan
     * 2. dimensi matrix > DIMENSION_THRESHOLD, langkah-langkah tidak dimunculkan
     *
     * @param augmented
     * @return SPLResult object yang terdiri dari solusi dan langkah-langkah
     */
    public static SPLResult gaussJordan(Matrix augmented){
        if (augmented.getRowsCount() > DIMENSION_THRESHOLD) {
            Matrix solution = gaussJordanWithoutSteps(augmented);
            return new SPLResult(solution, "Langkah-langkah tidak ditampilkan pada matriks besar.");
        }

        StringBuilder steps = new StringBuilder("Menyelesaikan SPL dengan Eliminasi Gauss-Jordan:\n\n");
        SPLResult rrefResult = reducedEchelonFormWithSteps(augmented);
        steps.append(rrefResult.steps);
        Matrix m = rrefResult.solution;

        steps.append("\nAnalisis Solusi dari matriks RREF:\n");
        Matrix a = m.removeLastCol();
        double[] b = m.getCol(m.getColsCount() - 1);

        if(checkNoSolution(a, b)){
            steps.append("-> Ditemukan baris [0 0 ... | c] dengan c != 0. SPL tidak memiliki solusi.\n");
            return new SPLResult(null, steps.toString());
        } else if (checkUniqueSolution(a, b)){
            // Jika solusi unik, langsung hitung dengan substitusi balik
            steps.append("-> Setiap variabel adalah variabel pivot. SPL memiliki solusi unik.\n");
            Matrix solution = backSubstitute(a, b);
            steps.append("Solusi:\n").append(solution);
            return new SPLResult(solution, steps.toString());
        } else {
            // Jika tidak keduanya, berarti solusi banyak.
            // Panggil fungsi untuk membuat matriks parametrik.
            steps.append("-> Terdapat variabel bebas. SPL memiliki banyak solusi (parametrik).\n");
            Matrix solution = getParametricSolution(a, b);
            steps.append("Solusi Parametrik (kolom pertama adalah konstanta, kolom berikutnya adalah parameter):\n").append(solution);
            return new SPLResult(solution, steps.toString());
        }
    }

    /*
     * Menyelesaikan SPL menggunakan metode cramer (Ax = b)
     * Behaviors :
     * 1. dimensi matrix <= DIMENSION_THRESHOLD, langkah-langkah akan dimunculkan
     * 2. dimensi matrix > DIMENSION_THRESHOLD, langkah-langkah tidak dimunculkan
     *
     * @param coeffMatrix -> A
     * @param constMatrix -> b
     * @return SPLResult object yang terdiri dari solusi dan langkah-langkah
     */
    public static SPLResult cramer(Matrix coeffMatrix, Matrix constMatrix) {
        if (coeffMatrix.getRowsCount() != coeffMatrix.getColsCount()) {
            throw new IllegalArgumentException("Matrix koefisien harus persegi untuk metode Cramer.");
        }
        if (constMatrix.getRowsCount() != coeffMatrix.getRowsCount() || constMatrix.getColsCount() != 1) {
            throw new IllegalArgumentException("Matrix konstanta harus berukuran n×1 dan memiliki jumlah baris yang sama dengan matrix koefisien.");
        }

        double coeffMatDet = Determinant.detReduksiBaris(coeffMatrix).value;
        if (coeffMatDet == 0) {
            throw new IllegalArgumentException("Det == 0, metode cramer tidak bisa diimplementasikan.");
        }
        int n = coeffMatrix.getColsCount(); // number of variables
        Matrix determinants = new Matrix(n, 1);

        if (coeffMatrix.getRowsCount() > DIMENSION_THRESHOLD) {
            // apply cramer's rule
            for (int i = 0; i < n; i++) {
                Matrix replacedCol = coeffMatrix.replaceCol(i, constMatrix);
                determinants.setElmt(i, 0, Determinant.detReduksiBaris(replacedCol).value);
            }
            // look for each
            Matrix solution =  MatrixOperator.scalarDivision(determinants, coeffMatDet);
            return new SPLResult(solution, "Langkah-langkah tidak ditampilkan untuk Kaidah Cramer pada matriks besar.");
        }

        StringBuilder steps = new StringBuilder("Menyelesaikan SPL dengan Kaidah Cramer: xi = det(Ai) / det(A)\n\n");
        steps.append("LANGKAH 1: Menghitung determinan matriks koefisien (A)...\n");
        steps.append("Hasil: det(A) = ").append(String.format("%.3f", coeffMatDet)).append("\n\n");


        // apply cramer's rule
        for (int i = 0; i < n; i++) {
            steps.append("LANGKAH ").append(i + 2).append(": Hitung det(A").append(i + 1).append(").\n");
            Matrix replacedCol = coeffMatrix.replaceCol(i, constMatrix);
            steps.append("Matriks A").append(i + 1).append(" (kolom ").append(i + 1).append(" diganti):\n").append(replacedCol).append("\n");

            double determinant = Determinant.detReduksiBaris(replacedCol).value;
            steps.append("Hasil: det(A").append(i + 1).append(") = ").append(String.format("%.3f", determinant)).append("\n\n");
            determinants.setElmt(i, 0, determinant);
        }
        // look for each
        Matrix solution =  MatrixOperator.scalarDivision(determinants, coeffMatDet);
        steps.append("Solusi akhir:\n").append(solution);
        return new SPLResult(solution, steps.toString());
    }

    /*
     * Menyelesaikan SPL menggunakan metode invers
     * Behaviors :
     * 1. dimensi matrix <= DIMENSION_THRESHOLD, langkah-langkah akan dimunculkan
     * 2. dimensi matrix > DIMENSION_THRESHOLD, langkah-langkah tidak dimunculkan
     *
     * @param coeffMatrix
     * @param constMatrix
     * @return SPLResult object yang terdiri dari solusi dan langkah-langkah
     */
    public static SPLResult inverseMethod(Matrix coef, Matrix constantM) {
        if (coef.getRowsCount() != coef.getColsCount()) {
            throw new IllegalArgumentException("Matrix A harus persegi");
        }
        // intinya cuma cari invers: A-1, terus X = A-1 . B

        try {
            if (coef.getRowsCount() > DIMENSION_THRESHOLD) {
                InverseResult inverseRes = Inverse.inverseAugment(coef);
                Matrix solution =  MatrixOperator.matrixMultiplication(inverseRes.matrix, constantM);
                return new SPLResult(solution, "Langkah-langkah tidak ditampilkan untuk Metode Invers pada matriks besar.");
            }

            StringBuilder steps = new StringBuilder("Menyelesaikan SPL dengan Metode Matriks Balikan: X = A⁻¹ * B\n\n");
            steps.append("LANGKAH 1: Cari matriks balikan (A⁻¹).\n");

            InverseResult inverseResult = Inverse.inverseAugment(coef);
            steps.append(inverseResult.steps);
            Matrix inverseRes = inverseResult.matrix;

            steps.append("\nLANGKAH 2: Kalikan A⁻¹ dengan B untuk mendapatkan X.\n");
            steps.append("X = A⁻¹ * B\n").append(inverseRes).append("\n   *\n").append(constantM).append("\n");

            Matrix solution = MatrixOperator.matrixMultiplication(inverseRes, constantM);
            steps.append("   =\n").append(solution);
            return new SPLResult(solution, steps.toString());

        } catch (IllegalArgumentException error) {
            return new SPLResult(null, error.getMessage());
        }
    }

    /*
     * Mengubah matriks augmented menjadi bentuk eselon baris (Row Echelon Form)
     * menggunakan eliminasi Gauss, sambil mencatat setiap langkah Operasi Baris Elementer.
     * @param augmented Matriks augmented [A|b] yang akan diubah.
     * @return Objek SPLResult yang berisi matriks hasil dalam bentuk eselon baris dan string rincian langkahnya.
     */
    public static SPLResult echelonFormWithSteps(Matrix augmented) {
        StringBuilder steps = new StringBuilder();
        Matrix m = augmented.copyMatrix();
        int rowCount = m.getRowsCount();
        int colCount = m.getColsCount();
        int pivotRow = 0;
        for (int pivotCol = 0; pivotCol < colCount - 1 && pivotRow < rowCount; pivotCol++) {
            int nonZero = pivotRow;
            while (nonZero < rowCount && Math.abs(m.getElmt(nonZero, pivotCol)) < 1e-9) nonZero++;
            if (nonZero == rowCount) continue;
            if (nonZero != pivotRow) {
                steps.append(String.format("-> Tukar B%d dengan B%d\n", pivotRow + 1, nonZero + 1));
                m.swapRow(pivotRow, nonZero);
            }
            double pivotVal = m.getElmt(pivotRow, pivotCol);
            if (Math.abs(pivotVal - 1.0) > 1e-9) {
                steps.append(String.format("-> B%d = B%d / %.3f\n", pivotRow + 1, pivotRow + 1, pivotVal));
                m.scaleRow(pivotRow, 1.0 / pivotVal);
            }
            for (int i = pivotRow + 1; i < rowCount; i++) {
                double factor = m.getElmt(i, pivotCol);
                if (Math.abs(factor) > 1e-9) {
                    steps.append(String.format("-> B%d = B%d - (%.3f * B%d)\n", i + 1, i + 1, factor, pivotRow + 1));
                    m.addRowMultiple(i, pivotRow, -factor);
                }
            }
            steps.append("Matriks setelah proses di kolom pivot ").append(pivotCol + 1).append(":\n").append(m).append("\n\n");
            pivotRow++;
        }
        steps.append("Bentuk Eselon Baris tercapai.\n");
        return new SPLResult(m, steps.toString());
    }

    /*
     * Mengubah matriks menjadi bentuk eselon baris tereduksi (Reduced Row Echelon Form)
     * menggunakan eliminasi Gauss-Jordan, sambil mencatat setiap langkahnya.
     * Proses ini mencakup eliminasi maju (membuat segitiga bawah nol) dan eliminasi mundur (membuat segitiga atas nol).
     * @param m Matriks yang akan diubah ke bentuk RREF.
     * @return Objek SPLResult yang berisi matriks hasil dalam bentuk RREF dan string rincian langkahnya.
     */
    public static SPLResult reducedEchelonFormWithSteps(Matrix m){
        StringBuilder steps = new StringBuilder("Mengubah ke Bentuk Eselon Baris Tereduksi (RREF):\n");
        Matrix m1 = m.copyMatrix();
        steps.append("Matriks Awal:\n").append(m1).append("\n\n");
        int rowCount = m1.getRowsCount();
        int colCount = m1.getColsCount();

        int lead = 0;
        for(int r = 0; r < rowCount; r++){
            if (lead >= colCount){
                break;
            }
            int pivotRow = r;

            while (pivotRow < rowCount && m1.getElmt(pivotRow, lead) == 0){
                pivotRow++;
            }
            if (pivotRow == rowCount){
                lead++;
                r--;
                continue;
            }
            if (pivotRow != r){
                steps.append(String.format("-> Tukar B%d dengan B%d\n", r + 1, pivotRow + 1));
                m1.swapRow(r, pivotRow);
            }
            // normalisasi pivot jadi 1
            double pivotVal = m1.getElmt(r, lead);
            if (pivotVal != 0 && pivotVal != 1){
                steps.append(String.format("-> B%d = B%d / %.3f\n", r + 1, r + 1, pivotVal));
                m1.scaleRow(r, 1.0/pivotVal);
            }

            for (int i = r+1; i < rowCount; i++){
                double factor = m1.getElmt(i, lead);
                if (factor != 0){
                    steps.append(String.format("-> B%d = B%d - (%.3f * B%d)\n", i + 1, i + 1, factor, r + 1));
                    m1.addRowMultiple(i, r, -1 * factor);
                }
            }
            steps.append("Matriks setelah proses di kolom pivot ").append(lead + 1).append(":\n").append(m1).append("\n\n");
            lead++;
        }

        steps.append("\nMemulai fase eliminasi ke atas (Back Substitution):\n");
        for (int i = rowCount - 1; i >= 0; i--) {
            lead = -1;
            for (int j = 0; j < colCount; j++) {
                if (Math.abs(m1.getElmt(i, j)) > 1e-9) {
                    lead = j;
                    break;
                }
            }

            if (lead != -1) {
                for (int r = i - 1; r >= 0; r--) {
                    double factor = m1.getElmt(r, lead);
                    if (Math.abs(factor) > 1e-9) {
                        steps.append(String.format("-> B%d = B%d - (%.3f * B%d)\n", r + 1, r + 1, factor, i + 1));
                        m1.addRowMultiple(r, i, -factor);
                    }
                }
            }
        }
        steps.append("Matriks setelah eliminasi ke atas:\n").append(m1).append("\n\n");
        steps.append("Bentuk Eselon Baris Tereduksi (RREF) akhir tercapai.\n");
        return new SPLResult(m1, steps.toString());
    }

    /*
     * Menyelesaikan Sistem Persamaan Linear (SPL) menggunakan metode eliminasi Gauss
     * yang diikuti dengan substitusi balik (back substitution), tanpa mencatat langkah-langkahnya.
     * @param augmented Matriks augmented [A|b] dari sistem yang akan diselesaikan.
     * @return Sebuah matriks kolom (vektor) yang merepresentasikan solusi unik dari SPL.
     */
    public static Matrix gaussWithoutSteps(Matrix augmented) {
        Matrix m = augmented.copyMatrix();

        int rowCount = m.getRowsCount();
        int colCount = m.getColsCount();
        int pivotRow = 0;

        // colCount - 1 karena [A | b]
        for (int pivotCol = 0; pivotCol < colCount - 1 && pivotRow < rowCount; pivotCol++) {
            int nonZero = pivotRow;

            // iterate column
            while (nonZero < rowCount && m.getElmt(nonZero, pivotCol) == 0) {
                nonZero++;
            }
            if (nonZero == rowCount) {
                continue;  // no 0 value, variabel bebas
            }

            if (nonZero != pivotRow) {
                m.swapRow(pivotRow, nonZero);
            }

            double pivotVal = m.getElmt(pivotRow, pivotCol);
            if (pivotVal != 1 && pivotVal != 0) {
                m.scaleRow(pivotRow, 1.0 / pivotVal);
            } // pivot is now 1

            m = zeroBelowPivot(m, pivotRow, pivotCol); // i didnt make this one, intinya smua angka dibawah pivot jadi 0

            pivotRow++;
        }

        Matrix A = m.removeLastCol();  // [A | b]
        double[] b = m.getCol(colCount - 1);

        return backSubstitute(A, b);
    }

    /*
     * Menyelesaikan SPL menggunakan metode eliminasi Gauss-Jordan tanpa mencatat langkah.
     * Fungsi ini akan mengubah matriks ke RREF, lalu menganalisis dan mengembalikan jenis solusinya
     * (unik, banyak solusi, atau tidak ada solusi).
     * @param augmented Matriks augmented [A|b] dari sistem yang akan diselesaikan.
     * @return Sebuah matriks solusi. Bisa berupa vektor kolom (solusi unik), matriks parametrik (banyak solusi), atau null (tidak ada solusi).
     */
    private static Matrix gaussJordanWithoutSteps(Matrix augmented){
        Matrix m = augmented.copyMatrix();
        m = reducedEchelonFormWithoutSteps(m);

        Matrix a = m.removeLastCol();
        double[] b = m.getCol(m.getColsCount() - 1);

        if(checkNoSolution(a, b)){
            return null;
        } else if (checkUniqueSolution(a, b)){
            return backSubstitute(a,b);
        } else {
            return getParametricSolution(a, b);
        }
    }

    /*
     * Membangun matriks yang merepresentasikan solusi parametrik untuk sistem dengan banyak solusi.
     * Kolom pertama dari matriks hasil adalah solusi khusus (particular solution),
     * sementara kolom-kolom berikutnya adalah vektor yang berkorespondensi dengan setiap variabel bebas.
     * @param a Matriks koefisien yang sudah dalam bentuk RREF.
     * @param b Vektor konstanta yang sudah disesuaikan dengan matriks RREF.
     * @return Matriks yang merepresentasikan solusi parametrik.
     */
    public static Matrix getParametricSolution(Matrix a, double[] b) {
        int n = a.getColsCount();
        boolean[] isPivot = new boolean[n];
        int[] pivotRow = new int[n];
        java.util.Arrays.fill(pivotRow, -1);

        // deteksi pivot (pertama non-zero di tiap baris)
        for (int i = 0; i < a.getRowsCount(); i++) {
            for (int j = 0; j < n; j++) {
                if (Math.abs(a.getElmt(i, j)) > 1e-9) {
                    isPivot[j] = true;
                    pivotRow[j] = i;
                    break;
                }
            }
        }

        // cari variabel bebas
        java.util.ArrayList<Integer> freeVars = new java.util.ArrayList<>();
        for (int j = 0; j < n; j++) {
            if (!isPivot[j]) freeVars.add(j);
        }

        int m = freeVars.size();
        Matrix result = new Matrix(n, 1 + m);

        // inisialisasi semua 0
        double[][] param = new double[n][1 + m];
        for (int i = 0; i < n; i++) java.util.Arrays.fill(param[i], 0.0);

        // set variabel bebas (tiap parameter berdiri sendiri)
        for (int p = 0; p < m; p++) {
            int freeIdx = freeVars.get(p);
            param[freeIdx][p + 1] = 1.0; // x_free = 1 * parameter
        }

        // substitusi balik dari bawah ke atas
        for (int i = a.getRowsCount() - 1; i >= 0; i--) {
            // cari pivot di baris i
            int pivotCol = -1;
            for (int j = 0; j < n; j++) {
                if (Math.abs(a.getElmt(i, j)) > 1e-9) {
                    pivotCol = j;
                    break;
                }
            }
            if (pivotCol == -1) continue; // baris nol

            double rhs = b[i];
            double[] coeff = new double[n];
            for (int j = 0; j < n; j++) coeff[j] = a.getElmt(i, j);

            // x_pivot = rhs - sum(coeff[j]*xj)
            double[] current = new double[1 + m];
            current[0] = rhs;
            for (int j = pivotCol + 1; j < n; j++) {
                if (Math.abs(coeff[j]) > 1e-9) {
                    for (int k = 0; k < 1 + m; k++) {
                        current[k] -= coeff[j] * param[j][k];
                    }
                }
            }

            // simpan hasil
            for (int k = 0; k < 1 + m; k++) {
                param[pivotCol][k] = current[k];
            }
        }

        // salin ke Matrix
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < 1 + m; j++) {
                result.setElmt(i, j, param[i][j]);
            }
        }

        return result;
    }



    /*
     * Fungsi pembantu untuk membuat semua elemen di bawah sebuah pivot menjadi nol dalam satu kolom.
     * Ini adalah bagian dari proses eliminasi maju dalam eliminasi Gauss.
     * @param m Matriks yang sedang diproses.
     * @param pivotRow Indeks baris dari pivot.
     * @param pivotCol Indeks kolom dari pivot.
     * @return Matriks baru yang elemen di bawah pivotnya sudah dinolkan.
     */
    public static Matrix zeroBelowPivot(Matrix m, int pivotRow, int pivotCol){
        Matrix m1 = m.copyMatrix();
        for(int i = pivotRow + 1; i < m1.getRowsCount(); i++){
            double factor = m1.getElmt(i, pivotCol);
            if (factor != 0){
                m1.addRowMultiple(i, pivotRow, -1 * factor);
            }
        }
        return m1;
    }

    /*
     * Mengubah matriks yang diberikan menjadi bentuk eselon baris tereduksi (Reduced Row Echelon Form)
     * tanpa mencatat langkah-langkah prosesnya.
     * @param m Matriks yang akan diubah.
     * @return Matriks baru dalam bentuk RREF.
     */
    public static Matrix reducedEchelonFormWithoutSteps(Matrix m){
        Matrix m1 = m.copyMatrix();
        int rowCount = m1.getRowsCount();
        int colCount = m1.getColsCount();

        int lead = 0;
        for(int r = 0; r < rowCount; r++){
            if (lead >= colCount){
                return m1;
            }
            int pivotRow = r;

            while (pivotRow < rowCount && m1.getElmt(pivotRow, lead) == 0){
                pivotRow++;
            }
            if (pivotRow == rowCount){
                lead++;
                r--;
                continue;
            }
            if (pivotRow != r){
                m1.swapRow(r, pivotRow);
            }
            // normalisasi pivot jadi 1
            double pivotVal = m1.getElmt(r, lead);
            if (pivotVal != 0 && pivotVal != 1){
                m1.scaleRow(r, 1.0/pivotVal);
            }

            for (int i = r+1; i < rowCount; i++){
                double factor = m1.getElmt(i, lead);
                if (factor != 0){
                    m1.addRowMultiple(i, r, -1 * factor);
                }
            }
            lead++;
        }

        for(int i = rowCount-1; i >= 0; i--){
            int leadingOneCol = -1;
            for(int j = 0; j < colCount; j++){
                if (m1.getElmt(i,j) == 1){
                    leadingOneCol = j;
                    break;
                } else if (m1.getElmt(i,j) != 0){
                    break;
                }
            }
            if (leadingOneCol != -1){
                for(int r = i-1; r >= 0; r--){
                    double factor = m1.getElmt(r, leadingOneCol);
                    if (factor != 0){
                        m1.addRowMultiple(r, i, -1 * factor);
                    }
                }
            }
        }
        return m1;
    }

    /*
     * Fungsi pembantu untuk mencari indeks kolom dari elemen non-nol pertama dalam sebuah baris.
     * @param row Sebuah array double yang merepresentasikan satu baris matriks.
     * @return Indeks elemen non-nol pertama, atau -1 jika seluruh baris berisi nol.
     */
    private static int idxNotZero(double[] row) {
        for (int i = 0; i < row.length; i++) {
            if (row[i] != 0) return i;
        }
        return -1;
    }

    /*
     * Memeriksa apakah sebuah SPL tidak memiliki solusi.
     * Ini dideteksi dengan adanya baris kontradiktif pada matriks eselon,
     * yaitu baris berbentuk [0 0 ... 0 | k] di mana k bukan nol.
     * @param a Matriks koefisien dalam bentuk eselon.
     * @param b Vektor konstanta yang sudah disesuaikan.
     * @return true jika tidak ada solusi, false jika sebaliknya.
     */
    public static boolean checkNoSolution(Matrix a, double[] b) {
        for (int i = 0; i < a.getRowsCount(); i++) {
            boolean isRowAllZero = true;
            // Check if the entire row in the coefficient matrix is zero
            for (int j = 0; j < a.getColsCount(); j++) {
                if (a.getElmt(i, j) != 0) {
                    isRowAllZero = false;
                    break;
                }
            }
            // check if 0 = non-zero value
            if (isRowAllZero && b[i] != 0) {
                return true;
            }
        }
        return false;
    }

    /*
     * Memeriksa apakah sebuah SPL memiliki banyak solusi (solusi tak hingga).
     * Ini terjadi jika sistem konsisten (tidak ada kontradiksi) dan jumlah variabel
     * lebih banyak dari rank matriks (terdapat variabel bebas).
     * @param a Matriks koefisien dalam bentuk eselon.
     * @param b Vektor konstanta yang sudah disesuaikan.
     * @return true jika terdapat banyak solusi, false jika sebaliknya.
     */
    public static boolean checkManySolution(Matrix a, double[] b) {
        if (checkNoSolution(a, b)) {
            return false;
        }

        // The rank of a matrix in row echelon form is its number of non-zero rows.
        int rank = 0;
        for (int i = 0; i < a.getRowsCount(); i++) {
            for (int j = 0; j < a.getColsCount(); j++) {
                if (a.getElmt(i, j) != 0) {
                    rank++;
                    break;
                }
            }
        }

        int n = a.getColsCount();
        return rank < n;
    }

    /*
     * Memeriksa apakah sebuah SPL memiliki solusi unik.
     * Ini terjadi jika sistem konsisten dan rank matriks koefisien sama dengan jumlah variabel
     * (tidak ada variabel bebas).
     * @param a Matriks koefisien dalam bentuk eselon.
     * @param b Vektor konstanta yang sudah disesuaikan.
     * @return true jika solusi unik, false jika sebaliknya.
     */
    public static boolean checkUniqueSolution(Matrix a, double[] b) {
        int rankA = 0;
        for (int i = 0; i < a.getRowsCount(); i++) {
            boolean nonZero = false;
            for (int j = 0; j < a.getColsCount(); j++) {
                if (Math.abs(a.getElmt(i, j)) > 1e-9) {
                    nonZero = true;
                    break;
                }
            }
            if (nonZero) rankA++;
        }

        // Hitung rank matriks augmented (A|b)
        int rankAb = 0;
        for (int i = 0; i < a.getRowsCount(); i++) {
            boolean nonZero = false;
            for (int j = 0; j < a.getColsCount(); j++) {
                if (Math.abs(a.getElmt(i, j)) > 1e-9) {
                    nonZero = true;
                    break;
                }
            }
            // kalau di kolom A semua nol, tapi b[i] != 0, tetap baris tak nol
            if (!nonZero && Math.abs(b[i]) > 1e-9) {
                nonZero = true;
            }
            if (nonZero) rankAb++;
        }

        int n = a.getColsCount();

        // Tidak ada solusi jika rank(A|b) > rank(A)
        if (rankAb > rankA) {
            return false;
        }

        // Solusi unik hanya jika rank(A) == jumlah variabel
        return rankA == n;
    }



    /*
     * Menjalankan proses substitusi balik (backward substitution) untuk menemukan solusi SPL
     * dari matriks yang sudah dalam bentuk eselon baris.
     * @param A Matriks koefisien dalam bentuk eselon baris.
     * @param B Vektor konstanta yang sudah disesuaikan.
     * @return Sebuah matriks kolom (vektor) yang berisi nilai-nilai solusi.
     */
    private static Matrix backSubstitute(Matrix A, double[] B) {
        int n = A.getColsCount();
        Matrix solution = new Matrix(n, 1);

        for (int i = A.getRowsCount() - 1; i >= 0; i--) {
            int idx = idxNotZero(A.getRow(i));
            if (idx == -1) {
                continue;
            }
            double sum = 0;
            for (int j = idx + 1; j < n; j++) {
                sum += A.getElmt(i, j) * solution.getElmt(j, 0);
            }
            double xi = (B[i] - sum) / A.getElmt(i, idx);
            solution.setElmt(idx, 0, xi);
        }
        return solution;
    }

}
