package algeo.gui;

import algeo.modules.*;
import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.scene.control.Label;
import javafx.scene.control.TextArea;
import javafx.stage.Stage;

/*
 * Controller yang bertanggung jawab untuk menangani semua aksi
 * dari file CalculationUI.fxml. Ini adalah controller umum untuk
 * SPL, Determinan, dan Invers.
 */
public class CalculationController {

    @FXML private Label headerLabel;
    @FXML private TextArea inputArea;
    @FXML private TextArea outputArea;
    @FXML private TextArea stepsArea;

    private UIController uiController;
    private Stage primaryStage;
    private String type;
    private String method;
    private Runnable backAction;

    public void initData(String type, String method, UIController uiController, Stage stage, Runnable backAction) {
        this.type = type;
        this.method = method;
        this.uiController = uiController;
        this.primaryStage = stage;
        this.backAction = backAction;
        this.headerLabel.setText(type + " - " + method);
    }

    @FXML
    void handleCalculate(ActionEvent event) {
        try {
            Matrix inputMatrix = MatrixParser.parseMatrix(inputArea.getText());
            String resultText = "";
            String stepsText = "Langkah-langkah tidak diimplementasikan.";

            switch (this.type) {
                case "SPL":
                    SPLResult splResult = solveSPL(inputMatrix);
                    resultText = FormatResult.formatSolutionResult(splResult.solution);
                    stepsText = splResult.steps;
                    break;
                case "Determinan":
                    DeterminantResult detResult = solveDeterminant(inputMatrix);
                    resultText = "Nilai Determinan: " + String.format("%.3f", detResult.value);
                    stepsText = detResult.steps;
                    break;
                case "Invers":
                    InverseResult invResult = solveInverse(inputMatrix);
                    resultText = "Matriks Invers:\n" + FormatResult.matrixToString(invResult.matrix);
                    stepsText = invResult.steps;
                    break;
            }
            outputArea.setText(resultText);
            stepsArea.setText(stepsText);

        } catch (Exception ex) {
            uiController.showErrorDialog("Error Perhitungan", "Terjadi kesalahan: " + ex.getMessage());
        }
    }

    @FXML
    void handleLoadFile(ActionEvent event) {
        uiController.loadFileToTextArea(inputArea);
    }

    @FXML
    void handleSaveFile(ActionEvent event) {
        String contentToSave = "Metode: " + headerLabel.getText() + "\n\n" +
                "Input:\n" + inputArea.getText() + "\n\n" +
                "Hasil:\n" + outputArea.getText();
        uiController.saveTextToFile(contentToSave);
    }

    @FXML
    void handleBack(ActionEvent event) {
        if (backAction != null) {
            backAction.run();
        }
    }

    private SPLResult solveSPL(Matrix augmentedMatrix) {
        switch (this.method) {
            case "Eliminasi Gauss":
                return SPL.gauss(augmentedMatrix);
            case "Metode Matriks Balikan": {
                Matrix[] ab = separateAugmentedMatrix(augmentedMatrix);
                return SPL.inverseMethod(ab[0], ab[1]);
            }
            case "Kaidah Cramer": {
                Matrix[] ab = separateAugmentedMatrix(augmentedMatrix);
                return SPL.cramer(ab[0], ab[1]);
            }
            default:
                return SPL.gaussJordan(augmentedMatrix);
        }
    }

    private DeterminantResult solveDeterminant(Matrix matrix) {
        if ("Ekspansi Kofaktor".equals(this.method)) {
            return Determinant.detCofactor(matrix);
        } else {
            return Determinant.detReduksiBaris(matrix);
        }
    }

    private InverseResult solveInverse(Matrix matrix) {
        if ("Metode Adjoint".equals(this.method)) {
            return Inverse.inverseAdjoin(matrix);
        } else {
            return Inverse.inverseAugment(matrix);
        }
    }

    private Matrix[] separateAugmentedMatrix(Matrix augmented) {
        int rows = augmented.getRowsCount();
        int cols = augmented.getColsCount();
        int numVars = cols - 1;
        Matrix a = augmented.subMatrix(0, rows - 1, 0, numVars - 1);
        Matrix b = augmented.subMatrix(0, rows - 1, numVars, numVars);
        return new Matrix[]{a, b};
    }
}
