package algeo.gui;

import javafx.geometry.Insets;
import javafx.geometry.Pos;
import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.layout.VBox;

/*
 * Kelas boundary yang bertanggung jawab untuk membuat semua scene menu
 */
public class MainMenuUI {
    private final UIController controller;

    public MainMenuUI(UIController controller) {
        this.controller = controller;
    }

    public Scene createMainMenuScene() {
        VBox layout = createMenuLayout("Lintasjava Calculator", "Menu Utama");
        Button btnSPL = createMenuButton("Sistem Persamaan Linear");
        Button btnDet = createMenuButton("Determinan Matriks");
        Button btnInv = createMenuButton("Invers Matriks");
        Button btnReg = createMenuButton("Regresi Linier Berganda");
        Button btnInt = createMenuButton("Interpolasi");
        Button btnExit = createExitButton();

        btnSPL.setOnAction(e -> controller.showSplMenu());
        btnDet.setOnAction(e -> controller.showDeterminantMenu());
        btnInv.setOnAction(e -> controller.showInverseMenu());
        btnReg.setOnAction(e -> controller.showRegressionUI());
        btnInt.setOnAction(e -> controller.showInterpolationUI());

        layout.getChildren().addAll(btnSPL, btnDet, btnInv, btnReg, btnInt, btnExit);
        return new Scene(layout, 500, 450);
    }

    public Scene createSplMenuScene() {
        VBox layout = createMenuLayout("Sistem Persamaan Linear", "Pilih metode:");
        Button btnGauss = createMenuButton("Metode Eliminasi Gauss");
        Button btnGaussJordan = createMenuButton("Metode Eliminasi Gauss-Jordan");
        Button btnInverse = createMenuButton("Metode Matriks Balikan");
        Button btnCramer = createMenuButton("Metode Kaidah Cramer");
        Button btnBack = createBackButton();
        layout.getChildren().addAll(btnGauss, btnGaussJordan, btnInverse, btnCramer, btnBack);

        btnGauss.setOnAction(e -> controller.showCalculationScene("SPL", "Eliminasi Gauss"));
        btnGaussJordan.setOnAction(e -> controller.showCalculationScene("SPL", "Eliminasi Gauss-Jordan"));
        btnInverse.setOnAction(e -> controller.showCalculationScene("SPL", "Metode Matriks Balikan"));
        btnCramer.setOnAction(e -> controller.showCalculationScene("SPL", "Kaidah Cramer"));
        return new Scene(layout, 500, 450);
    }

    public Scene createDeterminantMenuScene() {
        VBox layout = createMenuLayout("Determinan Matriks", "Pilih metode:");
        Button btnKofaktor = createMenuButton("Metode Ekspansi Kofaktor");
        Button btnReduksi = createMenuButton("Metode Reduksi Baris");
        Button btnBack = createBackButton();
        layout.getChildren().addAll(btnKofaktor, btnReduksi, btnBack);

        btnKofaktor.setOnAction(e -> controller.showCalculationScene("Determinan", "Ekspansi Kofaktor"));
        btnReduksi.setOnAction(e -> controller.showCalculationScene("Determinan", "Reduksi Baris"));
        return new Scene(layout, 500, 400);
    }

    public Scene createInverseMenuScene() {
        VBox layout = createMenuLayout("Invers Matriks", "Pilih metode:");
        Button btnAugment = createMenuButton("Metode Augment (Gauss-Jordan)");
        Button btnAdjoin = createMenuButton("Metode Adjoint");
        Button btnBack = createBackButton();
        layout.getChildren().addAll(btnAugment, btnAdjoin, btnBack);

        btnAugment.setOnAction(e -> controller.showCalculationScene("Invers", "Metode Augment"));
        btnAdjoin.setOnAction(e -> controller.showCalculationScene("Invers", "Metode Adjoint"));
        return new Scene(layout, 500, 400);
    }

    private VBox createMenuLayout(String title, String subtitle) {
        VBox layout = new VBox(15);
        layout.setPadding(new Insets(25));
        layout.setAlignment(Pos.TOP_CENTER);
        Label titleLabel = new Label(title);
        titleLabel.setStyle("-fx-font-size: 24px; -fx-font-weight: bold; -fx-padding: 0 0 10 0;");
        Label subtitleLabel = new Label(subtitle);
        subtitleLabel.setStyle("-fx-font-size: 16px; -fx-padding: 0 0 10 0;");
        layout.getChildren().addAll(titleLabel, subtitleLabel);
        return layout;
    }

    private Button createMenuButton(String text) {
        Button btn = new Button(text);
        btn.setMaxWidth(Double.MAX_VALUE);
        btn.setMinHeight(40);
        return btn;
    }

    private Button createBackButton() {
        Button btn = createMenuButton("Kembali ke Menu Utama");
        btn.setOnAction(e -> controller.showMainMenu());
        return btn;
    }

    private Button createExitButton() {
        Button btn = new Button("Keluar");
        btn.setMaxWidth(Double.MAX_VALUE);
        btn.setStyle("-fx-background-color: #f44336; -fx-text-fill: white;");
        btn.setOnAction(e -> controller.exit());
        return btn;
    }
}