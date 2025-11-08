package algeo.gui;

import algeo.modules.Matrix;
import algeo.modules.Regression;
import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.scene.control.TextArea;
import javafx.scene.control.TextField;
import javafx.stage.FileChooser;
import javafx.stage.Stage;

import java.nio.file.Files;
import java.io.File;

public class RegressionController {

    @FXML private TextArea infoArea;
    @FXML private TextArea outputArea;
    @FXML private TextField predictField;

    private UIController uiController;
    private Stage primaryStage;

    private Matrix coefficients;
    private FileHandler.RegressionInput regInput;

    public void setUiController(UIController uiController, Stage primaryStage) {
        this.uiController = uiController;
        this.primaryStage = primaryStage;
    }

    @FXML
    void handleLoadFile(ActionEvent event) {
        FileChooser fileChooser = new FileChooser();
        fileChooser.getExtensionFilters().add(new FileChooser.ExtensionFilter("Text Files", "*.txt"));
        File file = fileChooser.showOpenDialog(primaryStage);

        if (file != null) {
            try {
                String content = Files.readString(file.toPath());
                infoArea.setText(content);
                outputArea.clear();
                coefficients = null;
            } catch (Exception ex) {
                uiController.showErrorDialog("Error File Regresi", "Gagal membaca file: " + ex.getMessage());
            }
        }
    }

    // Parse isi area teks (manual atau dari file) dan proses regresi
    @FXML
    void handleProcessRegression(ActionEvent event) {
    String content = infoArea.getText().trim();
    if (content.isEmpty()) {
        uiController.showErrorDialog("Error", "Input data tidak boleh kosong.");
        return;
    }

    try {
        regInput = FileHandler.parseRegresi(content);

        Matrix Xused = Regression.expandPolynomialMatrix(regInput.X, regInput.derajatPolim);
        coefficients = Regression.multiRegression(Xused, regInput.y);

        String equation = formatRegressionString(coefficients, regInput.X.getColsCount() - 1, regInput.derajatPolim);
        outputArea.setText(
            "Persamaan Regresi Polinomial Berganda:\n\n" +
            equation + "\n\n" +
            "Derajat polinom: " + regInput.derajatPolim
        );

    } catch (Exception ex) {
        uiController.showErrorDialog("Error Regresi", "Gagal memproses data: " + ex.getMessage());
    }
}

    @FXML
    void handleBack(ActionEvent event) {
        uiController.showMainMenu();
    }

    @FXML
    void handleSave(ActionEvent event) {
        if (uiController != null && !outputArea.getText().isEmpty()) {
            String contentToSave = "Metode: Regresi Polinomial Berganda\n\n" +
                    "Input Data:\n" + infoArea.getText() + "\n\n" +
                    "Hasil:\n" + outputArea.getText();
            uiController.saveTextToFile(contentToSave);
        } else if (uiController != null) {
            uiController.showErrorDialog("Simpan Gagal", "Tidak ada hasil untuk disimpan. Silakan proses data terlebih dahulu.");
        }
    }

    @FXML
    void handlePredict(ActionEvent event) {
        if (coefficients == null) {
            uiController.showErrorDialog("Error", "Belum ada hasil regresi. Silakan proses data terlebih dahulu.");
            return;
        }

        try {
            String text = predictField.getText().trim();
            if (text.isEmpty()) {
                uiController.showErrorDialog("Error", "Input nilai x tidak boleh kosong.");
                return;
            }

            String[] parts = text.split("\\s+");
            double[] xValues = new double[parts.length];
            for (int i = 0; i < parts.length; i++) {
                xValues[i] = Double.parseDouble(parts[i].replace(',', '.'));
            }

            double yPred = Regression.predict(coefficients, xValues, regInput.derajatPolim);

            outputArea.appendText("\n\nPrediksi untuk input (" + text + "):\n");
            outputArea.appendText("y = " + String.format("%.3f", yPred));

        } catch (Exception ex) {
            uiController.showErrorDialog("Error Prediksi", "Gagal menghitung prediksi: " + ex.getMessage());
        }
    }

    // ===== Helper =====
    private String formatRegressionString(Matrix coeffs, int k, int degree) {
        StringBuilder sb = new StringBuilder("y = ");
        String[] names = algeo.modules.Regression.generateFeatureNames(k, degree);

        for (int i = 0; i < coeffs.getRowsCount(); i++) {
            double c = coeffs.getElmt(i, 0);
            if (i == 0) {
                sb.append(String.format("%.3f", c));
            } else {
                if (c >= 0) sb.append(" + ");
                else sb.append(" - ");
                sb.append(String.format("%.3f", Math.abs(c)));
                if (i < names.length) sb.append(names[i]);
            }
        }
        return sb.toString();
    }
}
