package algeo.modules;

import java.util.Arrays;
import java.util.Formatter;
public class Matrix {
    private final int rows;
    private final int cols;
    private final double[][] data;


    // constructor
    public Matrix(int rows, int cols) {
        this.rows = rows;
        this.cols = cols;
        data = new double[rows][cols];
    }

    public Matrix(double[][] val) {  // used for copyMatrix, create matrix based on 2Ds array
        rows = val.length;
        cols = val[0].length;
        data = new double[rows][cols];
        for (int i = 0; i < rows; i++) {
            if (val[i].length != cols) {
                throw new IllegalArgumentException("Panjang baris harus sama.");
            }
            data[i] = Arrays.copyOf(val[i], cols);
        }
    }

    // getter
    public int getRowsCount() {
        return rows;
    }

    public int getColsCount() {
        return cols;
    }

    public double getElmt(int i, int j) {
        return data[i][j];
    }

    public double[] getRow(int r) {
        if (r < 0 || r >= rows) {
            throw new IndexOutOfBoundsException("Index row salah.");
        }
        return Arrays.copyOf(data[r], cols);
    }

    public double[] getCol(int c) {
        if (c < 0 || c >= cols) {
            throw new IndexOutOfBoundsException("Index col salah.");
        }
        double[] col = new double[rows];
        for (int i = 0; i < rows; i++) {
            col[i] = data[i][c];
        }
        return col;
    }

    // setter
    public void setElmt(int i, int j, double val) {
        data[i][j] = val;
    }

    public void setRow(int r, double[] values) {
        if (values.length != cols) {
            throw new IllegalArgumentException("Panjang array tidak sama dengan kolom");
        }
        data[r] = Arrays.copyOf(values, cols);
    }

    public void setCol(int c, double[] values) {
        if (values.length != rows) {
            throw new IllegalArgumentException("Panjang array tidak sama dengan baris");
        }
        for (int i = 0; i < rows; i++) {
            data[i][c] = values[i];
        }
    }

    // utility
    public void displayMatrix() {
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                System.out.printf("%12.3f", data[i][j]);
                if (j != cols - 1) {
                    System.out.print(" ");
                }
            }
            if (i != rows - 1) {
                System.out.println();
            }
        }
        System.out.println();
    }

    public Matrix copyMatrix() {
        return new Matrix(data);
    }

    public void swapRow(int i, int j) {
        double[] temp = data[i];
        data[i] = data[j];
        data[j] = temp;
    }

    // scale the specified row with a non 0 constant
    public void scaleRow(int row, double constant) {
        if (constant == 0) {
            throw new IllegalArgumentException("Konstan tidak boleh nol");
        }
        for (int i = 0; i < cols; i++) {
            data[row][i] *= constant;
        }
    }

    // return targetRow = targetRow + (constant * rowHelper)
    public void addRowMultiple(int targetRow, int rowHelper, double constant) {
        if (constant == 0) {
            throw new IllegalArgumentException("Konstan tidak boleh nol");
        }
        for (int i = 0; i < cols; i++) {
            data[targetRow][i] += constant * data[rowHelper][i];
        }
    }


    // replace all element in the specified column with values from a 1-column matrix
    public Matrix replaceCol(int targetCol, Matrix newValue) {
        if (rows != newValue.getRowsCount()) {
            throw new IllegalArgumentException("Both matrix must have equal row length");
        } else if (newValue.getColsCount() != 1) {
            throw new IllegalArgumentException("newValue matrix must be 1D-row matrix");
        }

        Matrix result = new Matrix(data);

        for (int i = 0; i < rows; i++) {
            result.setElmt(i, targetCol, newValue.getElmt(i, 0));
        }
        return result;
    }

    public boolean isSquare() {
        return rows == cols;
    }

    public Matrix transpose() {
        Matrix transposed = new Matrix(cols, rows);
        for (int i = 0; i < cols; i++) {
            for (int j = 0; j < rows; j++) {
                transposed.setElmt(i, j, getElmt(j, i));
            }
        }
        return transposed;
    }

    // return matrix identitas
    public static Matrix identity(int dimension) {
        Matrix identityMatrix = new Matrix(dimension, dimension);
        for (int i = 0; i < dimension; i++) {
            identityMatrix.setElmt(i, i, 1);
        }
        return identityMatrix;
    }

    public Matrix removeRowColMatrix(int row, int col) {
        if (row < 0 || row >= rows || col < 0 || col >= cols) {
            throw new IllegalArgumentException("Index baris dan/atau kolom salah.");
        }

        Matrix removedRowColMatrix = new Matrix(rows - 1, cols - 1);
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                if (i == row || j == col) {
                    continue;
                } else if (i < row && j < col) {
                    removedRowColMatrix.setElmt(i, j, data[i][j]);
                } else if (i < row && j > col) {
                    removedRowColMatrix.setElmt(i, j - 1, data[i][j]);
                } else if (i > row && j < col) {
                    removedRowColMatrix.setElmt(i - 1, j, data[i][j]);
                } else if (i > row && j > col) {
                    removedRowColMatrix.setElmt(i - 1, j - 1, data[i][j]);
                }
            }
        }
        return removedRowColMatrix;
    }

    public Matrix removeLastCol() {
        Matrix result = new Matrix(rows, cols - 1);
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols - 1; j++) {
                result.setElmt(i, j, data[i][j]);
            }
        }
        return result;
    }

    //Overrides the default toString method
    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        Formatter formatter = new Formatter(sb);

        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                formatter.format("%12.3f", data[i][j]);
            }
            if (i < rows - 1) {
                sb.append("\n");
            }
        }
        return sb.toString();
    }

    // join two matrices horizontally (not necessarily an augmented [A|b] form)
    public static Matrix augment(Matrix A, Matrix B) {
        if (A.getRowsCount() != B.getRowsCount()) {
            throw new IllegalArgumentException("Jumlah baris A dan B harus sama");
        }

        int rows = A.getRowsCount();
        int colA = A.getColsCount();
        int colB = B.getColsCount();
        int cols = colA + colB;
        Matrix augmented = new Matrix(rows, cols);

        // copy matrix A
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < colA; j++) {
                augmented.setElmt(i, j, A.getElmt(i, j));
            }
        }

        // copy matrix B
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < colB; j++) {
                augmented.setElmt(i, j + colA, B.getElmt(i, j));
            }
        }

        return augmented;
    }

    public Matrix subMatrix(int startRow, int endRow, int startCol, int endCol) {
        if (startRow < 0 || endRow >= rows || startCol < 0 || endCol >= cols) {
            throw new IllegalArgumentException("Indeks submatrix berada di luar batas.");
        }
        if (startRow > endRow || startCol > endCol) {
            throw new IllegalArgumentException("Indeks awal tidak boleh lebih besar dari indeks akhir.");
        }

        // Matrix baru
        int newRows = endRow - startRow + 1;
        int newCols = endCol - startCol + 1;
        Matrix result = new Matrix(newRows, newCols);

        // Copy yang perlu
        for (int i = 0; i < newRows; i++) {
            for (int j = 0; j < newCols; j++) {
                double value = data[startRow + i][startCol + j];
                result.setElmt(i, j, value);
            }
        }

        return result;
    }

}
