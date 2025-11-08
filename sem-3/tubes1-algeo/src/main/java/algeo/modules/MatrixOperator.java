package algeo.modules;

public class MatrixOperator {
    /*
     * return a matrix result of a scalar times a matrix
     */
    public static Matrix scalarMultiplication(Matrix a, double scalar) {
        int rowLength = a.getRowsCount();
        int colLength = a.getColsCount();
        Matrix result = new Matrix(rowLength, colLength);
        for (int i = 0; i < rowLength; i++) {
            for (int j = 0; j < colLength; j++) {
                result.setElmt(i, j, scalar * a.getElmt(i, j));
            }
        }
        return result;
    }

    /*
     * return a matrix result of a scalar divided a matrix
     */
    public static Matrix scalarDivision(Matrix a, double scalar) {
        if (scalar == 0) {
            throw new IllegalArgumentException("scalar must not be 0");
        }
        int rowLength = a.getRowsCount();
        int colLength = a.getColsCount();
        Matrix result = new Matrix(rowLength, colLength);
        for (int i = 0; i < rowLength; i++) {
            for (int j = 0; j < colLength; j++) {
                result.setElmt(i, j, a.getElmt(i, j) / scalar);
            }
        }
        return result;
    }

    /*
     * return a matrix result from a * b
     */
    public static Matrix matrixMultiplication(Matrix a, Matrix b) {
        int resultRowLength = a.getRowsCount();
        int resultColLength = b.getColsCount();
        int aColLength = a.getColsCount();
        if (aColLength != b.getRowsCount()) {
            throw new IllegalArgumentException("a's column must be equal to b's row");
        }
        Matrix result = new Matrix(resultRowLength, resultColLength);
        for (int i = 0; i < resultRowLength; i++) {
            for (int j = 0; j < resultColLength; j++) {
                double sum = 0;
                for (int k = 0; k < aColLength; k++) {
                    sum += a.getElmt(i, k) * b.getElmt(k, j);
                }
                result.setElmt(i, j, sum);
            }
        }
        return result;
    }
}
