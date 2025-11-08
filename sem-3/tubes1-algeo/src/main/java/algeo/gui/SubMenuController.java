package algeo.gui;

import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.layout.VBox;
import java.util.List;

/*
 * Controller untuk template SubMenu.fxml.
 * Tugasnya adalah menerima data dinamis dan mengaturnya di tampilan.
 */
public class SubMenuController {

    @FXML private Label titleLabel;
    @FXML private VBox buttonContainer; // VBox ini akan diisi tombol secara dinamis

    private UIController uiController;

    public void setUiController(UIController uiController) {
        this.uiController = uiController;
    }

    /*
     * Mengkonfigurasi menu dengan judul dan tombol yang spesifik.
     */
    public void configureMenu(String title, List<Button> buttons) {
        titleLabel.setText(title);
        buttonContainer.getChildren().clear();
        buttonContainer.getChildren().addAll(buttons);
    }

    @FXML
    void handleBackToMain(ActionEvent event) {
        if (uiController != null) {
            uiController.showMainMenu();
        }
    }
}