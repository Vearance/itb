package algeo.gui;

import algeo.modules.Matrix;

public class MatrixParser {

    /*
     * Melakukan parsing input menjadi objek Matrix
     * Mendukung angka desimal (e.g., 0.75) dan pecahan (e.g., 3/4)
     */
    public static Matrix parseMatrix(String text) throws IllegalArgumentException {
        String[] lines = text.trim().split("\\n");
        if (lines.length == 0 || lines[0].trim().isEmpty()) {
            throw new IllegalArgumentException("Input matriks tidak boleh kosong.");
        }

        String[] firstLineParts = lines[0].trim().split("\\s+");
        int cols = firstLineParts.length;
        double[][] data = new double[lines.length][cols];

        for (int i = 0; i < lines.length; i++) {
            String[] parts = lines[i].trim().split("\\s+");
            if (parts.length != cols) {
                throw new IllegalArgumentException("Jumlah kolom tidak konsisten pada baris " + (i + 1));
            }
            for (int j = 0; j < cols; j++) {
                data[i][j] = parseNumber(parts[j]);
            }
        }
        return new Matrix(data);
    }

    /*
     * Metode helper baru untuk mem-parsing sebuah string yang bisa berupa
     * desimal atau pecahan.
     */
    private static double parseNumber(String numberStr) {
        String sanitizedStr = numberStr.replace(',', '.');

        // Cek apakah string mengandung karakter /
        if (sanitizedStr.contains("/")) {
            String[] fractionParts = sanitizedStr.split("/");
            if (fractionParts.length != 2) {
                throw new NumberFormatException("Format pecahan tidak valid: " + numberStr);
            }
            double numerator = Double.parseDouble(fractionParts[0]);
            double denominator = Double.parseDouble(fractionParts[1]);

            if (denominator == 0) {
                throw new ArithmeticException("Penyebut dalam pecahan tidak boleh nol.");
            }
            return numerator / denominator;
        } else {
            return Double.parseDouble(sanitizedStr);
        }
    }
}