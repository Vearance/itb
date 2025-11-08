package algeo.gui;

import javafx.application.Platform;
import javafx.event.ActionEvent;
import javafx.fxml.FXML;

/*
 * Controller yang bertanggung jawab untuk menangani semua aksi
 * dari file MainMenu.fxml.
 */
public class MainMenuController {
    private UIController uiController;

    /*
     * Metode ini akan dipanggil oleh UIController untuk memberikan
     * referensi dirinya sendiri ke controller ini.
     */
    public void setUiController(UIController uiController) {
        this.uiController = uiController;
    }

    @FXML
    void showSplMenu(ActionEvent event) {
        uiController.showSplMenu();
    }

    @FXML
    void showDeterminantMenu(ActionEvent event) {
        uiController.showDeterminantMenu();
    }

    @FXML
    void showInverseMenu(ActionEvent event) {
        uiController.showInverseMenu();
    }

    @FXML
    void showInterpolationUI(ActionEvent event) {
        uiController.showInterpolationUI();
    }

    @FXML
    void showRegressionUI(ActionEvent event) {
        uiController.showRegressionUI();
    }

    @FXML
    void handleExit(ActionEvent event) {
        Platform.exit();
    }
}