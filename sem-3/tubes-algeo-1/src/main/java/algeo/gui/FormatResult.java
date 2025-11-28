package algeo.gui;

import algeo.modules.Matrix;
import java.util.Locale;

public class FormatResult {

    public static String matrixToString(Matrix m) {
        if (m == null) return "Matriks null atau tidak valid.";
        return m.toString();
    }

    public static String formatSolutionResult(Matrix solutionMatrix) {
        if (solutionMatrix == null) {
            return "Sistem Persamaan Linier tidak memiliki solusi.";
        }
        int numVariables = solutionMatrix.getRowsCount();
        int numCols = solutionMatrix.getColsCount();
        StringBuilder sb = new StringBuilder();

        if (numCols == 1) {
            sb.append("Solusi unik ditemukan:\n");
            for (int i = 0; i < numVariables; i++) {
                sb.append(String.format(Locale.US, "x%d = %.3f\n", i + 1, solutionMatrix.getElmt(i, 0)));
            }
        } else {
            sb.append("Terdapat banyak solusi (solusi parametrik):\n");
            String[] params = {"t", "s", "r", "p", "q"};
            for (int i = 0; i < numVariables; i++) {
                sb.append(String.format(Locale.US, "x%d = ", i + 1));
                double constant = solutionMatrix.getElmt(i, 0);
                boolean isFirstTerm = true;
                if (Math.abs(constant) > 1e-9 || numCols == 1) {
                    sb.append(String.format(Locale.US, "%.3f", constant));
                    isFirstTerm = false;
                }
                for (int j = 1; j < numCols; j++) {
                    double coeff = solutionMatrix.getElmt(i, j);
                    if (Math.abs(coeff) > 1e-9) {
                        String paramName = (j - 1 < params.length) ? params[j - 1] : "t" + (j);
                        if (!isFirstTerm) {
                            sb.append(coeff > 0 ? " + " : " - ");
                        } else if (coeff < 0) {
                            sb.append("-");
                        }
                        isFirstTerm = false;
                        double absCoeff = Math.abs(coeff);
                        if (Math.abs(absCoeff - 1.0) > 1e-9) {
                            sb.append(String.format(Locale.US, "%.3f", absCoeff));
                        }
                        sb.append(paramName);
                    }
                }
                if (isFirstTerm) sb.append("0.000");
                sb.append("\n");
            }
        }
        return sb.toString();
    }

    /*
     * Metode ini membentuk format fungsi hassil inteprolasi polinomial, dengan:
     * - Suku diurutkan dari pangkat terbesar
     * - Suku dengan koefisien 0 tidak ditampilkan
     * @param Matrix coeffs berisikan koefisien polynomial dari derajat tertinggi -> terendah
     */
    public static String buildPolynomialString(Matrix coeffs) {
        StringBuilder sb = new StringBuilder("y(x) = ");
        boolean isFirstTerm = true;

        for (int i = 0 ; i < coeffs.getRowsCount(); i++) {
            double c = coeffs.getElmt(i, 0);
            if (Math.abs(c) < 1e-6) {
                continue;
            }

            double absCoeff = Math.abs(c);
            if (isFirstTerm) {
                if (c < 0) {
                    sb.append("-");
                }
            } else {
                sb.append(c > 0 ? " + " : " - ");
            }

            if (Math.abs(absCoeff - 1.0) > 1e-9 || i == 0) {
                sb.append(String.format(Locale.US, "%.3f", absCoeff));
            }

            int pangkat = coeffs.getRowsCount() - 1 - i;

            if (pangkat == 1) {
                sb.append("x");
            } else if (pangkat > 1) {
                sb.append("x^").append(pangkat);
            }

            isFirstTerm = false;
        }

        if (isFirstTerm) {
            sb.append("0.000");
        }

        return sb.toString();
    }
}
