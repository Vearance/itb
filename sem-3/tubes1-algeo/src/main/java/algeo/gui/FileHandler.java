package algeo.gui;

import algeo.modules.Matrix;

import java.io.File;
import java.io.FileNotFoundException;
import java.util.Scanner;

public class FileHandler {

    public static RegressionInput parseRegresi(String inputText) {
        Scanner sc = new Scanner(inputText);
        int rowCount = 0;
        int colCount = -1;

        // Hitung jumlah baris dan kolom
        while (sc.hasNextLine()) {
            String line = sc.nextLine().trim();
            if (line.isEmpty()) continue;
            String[] parts = line.split("\\s+");
            if (colCount == -1) colCount = parts.length;
            rowCount++;
        }
        sc.close();

        int n = rowCount - 1; // baris terakhir = derajat polinom
        int k = colCount - 1; // kolom terakhir = y

        double[][] X = new double[n][k + 1];
        double[][] y = new double[n][1];
        int derajatPolim = 0;

        sc = new Scanner(inputText);
        int row = 0;
        while (sc.hasNextLine()) {
            String line = sc.nextLine().trim();
            if (line.isEmpty()) continue;
            String[] parts = line.split("\\s+");

            if (row == rowCount - 1) {
                derajatPolim = (int) Double.parseDouble(parts[0].replace(',', '.'));
                break;
            }

            X[row][0] = 1.0;  // kolom bias
            for (int j = 0; j < k; j++) {
                X[row][j + 1] = Double.parseDouble(parts[j].replace(',', '.'));
            }
            y[row][0] = Double.parseDouble(parts[k].replace(',', '.'));
            row++;
        }
        sc.close();

        return new RegressionInput(new Matrix(X), new Matrix(y), derajatPolim);
    }


    private static double parseNumber(String token) {
        token = token.trim().replace(',', '.');
        if (token.contains("/")) {
            String[] frac = token.split("/");
            if (frac.length == 2) {
                double pembilang = Double.parseDouble(frac[0]);
                double penyebut = Double.parseDouble(frac[1]);
                return pembilang / penyebut;
            } else {
                throw new NumberFormatException("Format pecahan tidak valid: " + token);
            }
        }
        return Double.parseDouble(token);
    }

    /*
     * Membaca seluruh isi file menjadi satu String,
     * lalu menyerahkan proses parsing sepenuhnya ke MatrixParser.
     */
    public static Matrix readMatrix(String filename) throws FileNotFoundException {
        File file = new File(filename);
        StringBuilder fileContent = new StringBuilder();

        try (Scanner scanner = new Scanner(file)) {
            while (scanner.hasNextLine()) {
                fileContent.append(scanner.nextLine()).append("\n");
            }
        }

        return MatrixParser.parseMatrix(fileContent.toString());
    }

    private static double[] parseRow(String line) {
        String[] parts = line.split("\\s+");
        double[] row = new double[parts.length];
        for (int i = 0; i < parts.length; i++) {
            row[i] = parseNumber(parts[i]);
        }
        return row;
    }

    public static class RegressionInput {
        public Matrix X;
        public Matrix y;
        public int derajatPolim;

        public RegressionInput(Matrix X, Matrix y, int derajatPolim) {
            this.X = X;
            this.y = y;
            this.derajatPolim = derajatPolim;
        }
    }
}