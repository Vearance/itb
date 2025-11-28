package algeo.gui;

import javafx.application.Application;
import javafx.stage.Stage;

/*
 * Kelas utama aplikasi
 */
public class App extends Application {

    public static void main(String[] args) {
        launch(args);
    }

    @Override
    public void start(Stage primaryStage) {
        primaryStage.setTitle("Lintasjava Calculator");
        UIController uiController = new UIController(primaryStage);
        uiController.showMainMenu();
        primaryStage.show();
    }
}
