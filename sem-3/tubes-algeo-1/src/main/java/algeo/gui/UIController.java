package algeo.gui;

import algeo.modules.Matrix;
import javafx.application.Platform;
import javafx.fxml.FXMLLoader;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.scene.control.TextArea;
import javafx.stage.FileChooser;
import javafx.stage.Stage;
import java.util.Scanner;

import java.io.File;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.Arrays;
import java.util.List;
import java.util.function.Consumer;

public class UIController {
    private final Stage primaryStage;
    private final FileChooser fileChooser = new FileChooser();

    public UIController(Stage primaryStage) {
        this.primaryStage = primaryStage;
        fileChooser.getExtensionFilters().add(new FileChooser.ExtensionFilter("Text Files", "*.txt"));
    }

    public void showMainMenu() {
        loadScene("/MainMenu.fxml", MainMenuController.class, c -> c.setUiController(this));
    }

    public void showSplMenu() {
        Button btnGauss = createMenuButton("Metode Eliminasi Gauss", e -> showCalculationScene("SPL", "Eliminasi Gauss"));
        Button btnGaussJordan = createMenuButton("Metode Eliminasi Gauss-Jordan", e -> showCalculationScene("SPL", "Eliminasi Gauss-Jordan"));
        Button btnInverse = createMenuButton("Metode Matriks Balikan", e -> showCalculationScene("SPL", "Metode Matriks Balikan"));
        Button btnCramer = createMenuButton("Metode Kaidah Cramer", e -> showCalculationScene("SPL", "Kaidah Cramer"));
        showSubMenu("Sistem Persamaan Linear", Arrays.asList(btnGauss, btnGaussJordan, btnInverse, btnCramer));
    }

    public void showDeterminantMenu() {
        Button btnKofaktor = createMenuButton("Metode Ekspansi Kofaktor", e -> showCalculationScene("Determinan", "Ekspansi Kofaktor"));
        Button btnReduksi = createMenuButton("Metode Reduksi Baris", e -> showCalculationScene("Determinan", "Reduksi Baris"));
        showSubMenu("Determinan Matriks", Arrays.asList(btnKofaktor, btnReduksi));
    }

    public void showInverseMenu() {
        Button btnAugment = createMenuButton("Metode Augment (Gauss-Jordan)", e -> showCalculationScene("Invers", "Metode Augment"));
        Button btnAdjoin = createMenuButton("Metode Adjoint", e -> showCalculationScene("Invers", "Metode Adjoint"));
        showSubMenu("Invers Matriks", Arrays.asList(btnAugment, btnAdjoin));
    }

    public void exit() {
        Platform.exit();
    }

    public void showCalculationScene(String type, String method) {
        try {
            FXMLLoader loader = new FXMLLoader(getClass().getResource("/CalculationUI.fxml"));
            Parent root = loader.load();

            Runnable backAction;
            switch (type) {
                case "SPL": backAction = this::showSplMenu; break;
                case "Determinan": backAction = this::showDeterminantMenu; break;
                case "Invers": backAction = this::showInverseMenu; break;
                default: backAction = this::showMainMenu; break;
            }

            CalculationController controller = loader.getController();
            controller.initData(type, method, this, this.primaryStage, backAction);

            primaryStage.setScene(new Scene(root));
        } catch (IOException e) {
            e.printStackTrace();
            showErrorDialog("Error", "Gagal memuat CalculationUI.fxml.");
        }
    }

    public void showInterpolationUI() {
        loadScene("/InterpolationUI.fxml", InterpolationController.class, c -> c.setUiController(this));
    }

    public void showRegressionUI() {
        loadScene("/RegressionUI.fxml", RegressionController.class, c -> c.setUiController(this, this.primaryStage));
    }

    // ========== HELPER ==========

    private void showSubMenu(String title, List<Button> menuButtons) {
        try {
            FXMLLoader loader = new FXMLLoader(getClass().getResource("/SubMenu.fxml"));
            Parent root = loader.load();
            SubMenuController controller = loader.getController();
            controller.setUiController(this);
            controller.configureMenu(title, menuButtons);
            primaryStage.setScene(new Scene(root));
        } catch (IOException e) {
            e.printStackTrace();
            showErrorDialog("Error", "Gagal memuat SubMenu.fxml.");
        }
    }

    private <T> void loadScene(String fxmlPath, Class<T> controllerClass, Consumer<T> controllerInitializer) {
        try {
            FXMLLoader loader = new FXMLLoader(getClass().getResource(fxmlPath));
            Parent root = loader.load();
            T controller = loader.getController();
            controllerInitializer.accept(controller);
            primaryStage.setScene(new Scene(root));
        } catch (IOException e) {
            e.printStackTrace();
            showErrorDialog("Error", "Gagal memuat file FXML: " + fxmlPath);
        }
    }

    private Button createMenuButton(String text, javafx.event.EventHandler<javafx.event.ActionEvent> handler) {
        Button btn = new Button(text);
        btn.setMaxWidth(Double.MAX_VALUE);
        btn.setMinHeight(40.0);
        btn.setOnAction(handler);
        return btn;
    }

    public void loadFileToTextArea(TextArea textArea) {
        fileChooser.setTitle("Buka File Matriks");
        File file = fileChooser.showOpenDialog(primaryStage);
        if (file != null) {
            try (Scanner scanner = new Scanner(file)) {
                // langsung baca isi mentah file
                StringBuilder rawContent = new StringBuilder();
                while (scanner.hasNextLine()) {
                    rawContent.append(scanner.nextLine()).append("\n");
                }
                textArea.setText(rawContent.toString());
            } catch (Exception ex) {
                showErrorDialog("Error Membaca File", "Terjadi kesalahan saat membaca file: " + ex.getMessage());
            }
        }
    }


    public void saveTextToFile(String content) {
        fileChooser.setTitle("Simpan Hasil Ke File");
        File file = fileChooser.showSaveDialog(primaryStage);
        if (file != null) {
            try (PrintWriter writer = new PrintWriter(file)) {
                writer.write(content);
            } catch (IOException e) {
                showErrorDialog("Error Menyimpan File", "Gagal menyimpan file: " + e.getMessage());
            }
        }
    }

    public void showErrorDialog(String title, String message) {
        javafx.scene.control.Alert alert = new javafx.scene.control.Alert(javafx.scene.control.Alert.AlertType.ERROR);
        alert.setTitle(title);
        alert.setHeaderText(null);
        alert.setContentText(message);
        alert.showAndWait();
    }
}