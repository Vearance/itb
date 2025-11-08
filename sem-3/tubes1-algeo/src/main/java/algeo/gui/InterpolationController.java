package algeo.gui;

import algeo.modules.Interpolation;
import algeo.modules.Matrix;
import javafx.beans.value.ChangeListener;
import javafx.beans.value.ObservableValue;
import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.scene.control.RadioButton;
import javafx.scene.control.TextArea;
import javafx.scene.control.TextField;
import javafx.scene.control.Toggle;
import javafx.scene.control.ToggleGroup;
import javafx.scene.layout.HBox;
import java.util.Locale;

public class InterpolationController {

    @FXML private ToggleGroup methodGroup;
    @FXML private TextArea inputArea;
    @FXML private TextField estimateField;
    @FXML private TextArea outputArea;
    @FXML private HBox estimateBox;

    private UIController uiController;

    /*
     * Metode initialize() ini akan otomatis dijalankan oleh JavaFX setelah
     * semua komponen FXML dimuat.
     */
    @FXML
    public void initialize() {
        methodGroup.selectedToggleProperty().addListener((observable, oldToggle, newToggle) -> {
            if (newToggle != null) {
                String selectedMethod = ((RadioButton) newToggle).getText();
                estimateBox.setVisible("Polinomial".equals(selectedMethod));
            }
        });
    }

    public void setUiController(UIController uiController) {
        this.uiController = uiController;
    }

    @FXML
    void handleLoadFile(ActionEvent event) {
        if (uiController != null) {
            uiController.loadFileToTextArea(inputArea);
        }
    }

    @FXML
    void handleSaveFile(ActionEvent event) {
        if (uiController != null) {
            RadioButton selectedRadioButton = (RadioButton) methodGroup.getSelectedToggle();
            String method = (selectedRadioButton != null) ? selectedRadioButton.getText() : "Tidak diketahui";

            String contentToSave = "Metode: Interpolasi " + method + "\n\n" +
                    "Input Titik:\n" + inputArea.getText() + "\n\n" +
                    "Hasil:\n" + outputArea.getText();
            uiController.saveTextToFile(contentToSave);
        }
    }

    @FXML
    void handleCalculate(ActionEvent event) {
        try {
            RadioButton selectedRadioButton = (RadioButton) methodGroup.getSelectedToggle();
            String method = selectedRadioButton.getText();

            Matrix points = MatrixParser.parseMatrix(inputArea.getText());
            if (points.getColsCount() != 2) {
                uiController.showErrorDialog("Error Input", "Setiap baris input harus berisi 2 nilai: x dan y.");
                return;
            }

            if ("Polinomial".equals(method)) {
                Matrix coeffs = Interpolation.polynomialInterpolation(points);
                String polynomial = FormatResult.buildPolynomialString(coeffs);

                double[] domain = findMinMaxX(points);
                double minX = domain[0];
                double maxX = domain[1];

                String result = "Persamaan Polinomial:\n" + polynomial;
                result += String.format(Locale.US, "\n\nDomain prediksi interpolasi yang valid: [%.3f, %.3f]", minX, maxX);

                boolean doEstimate = estimateField.getText() != null && !estimateField.getText().trim().isEmpty();
                if (doEstimate) {
                    double xVal = Double.parseDouble(estimateField.getText().replace(',', '.'));
                    double yVal = predictPolynomial(coeffs, xVal);
                    result += String.format(Locale.US, "\n\nHasil Taksiran:\nf(%.3f) = %.3f", xVal, yVal);
                }
                outputArea.setText(result);

            } else {
                Matrix[] segments = Interpolation.interpolasiSplinaBezierKubik(points);
                outputArea.setText(formatBezierResult(segments));
            }
        } catch(Exception ex) {
            uiController.showErrorDialog("Error Interpolasi", "Gagal melakukan interpolasi: " + ex.getMessage());
        }
    }

    @FXML
    void handleBack(ActionEvent event) {
        uiController.showMainMenu();
    }

    // ========== Helper ==========

    private double predictPolynomial(Matrix coeffs, double x) {
        double y = 0;
        for (int i = 0; i < coeffs.getRowsCount(); i++) {
            y += coeffs.getElmt(i, 0) * Math.pow(x, coeffs.getRowsCount() - 1 - i);
        }
        return y;
    }

    private String formatBezierResult(Matrix[] segments) {
        StringBuilder sb = new StringBuilder("Titik-titik Kontrol Kurva Splina Bezier Kubik:\n");
        sb.append("------------------------------------------\n");
        int pointCtr = 1;
        for (Matrix controlPoints : segments) {
            for (int j = 0; j < controlPoints.getRowsCount(); j++) {
                sb.append(String.format("  Titik Kontrol %d: (%.3f, %.3f)\n", pointCtr,
                        controlPoints.getElmt(j, 0), controlPoints.getElmt(j, 1)));
                pointCtr++;
            }
        }
        sb.append("------------------------------------------\n");
        return sb.toString();
    }

    private static double[] findMinMaxX(Matrix M) {
        double minX = M.getElmt(0, 0);
        double maxX = M.getElmt(0, 0);

        for (int i = 1; i < M.getRowsCount(); i++) {
            double x = M.getElmt(i, 0);
            if (x < minX) minX = x;
            if (x > maxX) maxX = x;
        }

        return new double[]{minX, maxX};
    }
}
