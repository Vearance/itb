from PyQt6.QtWidgets import (
    QWidget, QLabel, QPushButton, QVBoxLayout, QHBoxLayout,
    QFrame, QLineEdit, QComboBox, QDialog, QSizePolicy, QSpacerItem,
    QMessageBox 
)
from PyQt6.QtCore import Qt, pyqtSignal, QSize, QTimer
from PyQt6.QtGui import QIcon, QIntValidator

# --- 1. Kelas Pop-up Dialogs ---

class KonfirmasiRekeningDialog(QDialog):
    def __init__(self, ewallet_name, no_rekening, nominal_awal, deskripsi, parent=None):
        super().__init__(parent)
        self.setWindowTitle("Konfirmasi Rekening")
        self.setFixedSize(400, 300) 
        self.setWindowFlags(self.windowFlags() & ~Qt.WindowType.WindowContextHelpButtonHint) 
        self.initUI(ewallet_name, no_rekening, nominal_awal, deskripsi)
    
    def create_data_row(self, label_text, value_text):
        h_layout = QHBoxLayout()
        label = QLabel(label_text)
        value = QLabel(value_text)
        value.setObjectName("DataValue")
        value.setAlignment(Qt.AlignmentFlag.AlignRight)
        h_layout.addWidget(label)
        h_layout.addWidget(value)
        return h_layout

    def initUI(self, ewallet_name, no_rekening, nominal_awal, deskripsi):
        self.setStyleSheet("""
            QDialog { background-color: white; border-radius: 15px; }
            QLabel { color: black; font-size: 14px; }
            #HeaderTitle { font-size: 16px; font-weight: bold; color: #5873E8; }
            #DataValue { font-weight: 500; }
            QPushButton { border-radius: 8px; font-weight: bold; height: 35px; }
        """)

        main_layout = QVBoxLayout(self)
        main_layout.setContentsMargins(25, 20, 25, 20)
        main_layout.setSpacing(20)
        
        title = QLabel("Konfirmasi Rekening")
        title.setObjectName("HeaderTitle")
        title.setAlignment(Qt.AlignmentFlag.AlignCenter)
        main_layout.addWidget(title)
        
        data_frame = QFrame()
        data_layout = QVBoxLayout(data_frame)
        data_layout.setContentsMargins(0, 10, 0, 10)
        data_layout.setSpacing(10)

        data_layout.addLayout(self.create_data_row("E-wallet", ewallet_name))
        data_layout.addLayout(self.create_data_row("No. Rekening", no_rekening))
        data_layout.addLayout(self.create_data_row("Nominal Awal", f"Rp{int(nominal_awal):,}")) 
        data_layout.addLayout(self.create_data_row("Deskripsi", deskripsi))

        main_layout.addWidget(data_frame)
        main_layout.addStretch()

        button_layout = QHBoxLayout()
        button_layout.setSpacing(10)
        
        btn_kembali = QPushButton("Kembali")
        btn_kembali.setStyleSheet("QPushButton { background-color: white; color: #5873E8; border: 1px solid #C0C0C0; } QPushButton:hover { background-color: #F0F2FF; }")
        btn_kembali.clicked.connect(self.reject) 

        btn_lanjutkan = QPushButton("Lanjutkan")
        btn_lanjutkan.setStyleSheet("QPushButton { background-color: #5873E8; color: white; } QPushButton:hover { background-color: #4A63D0; }")
        btn_lanjutkan.clicked.connect(self.accept) 
        
        button_layout.addWidget(btn_kembali)
        button_layout.addWidget(btn_lanjutkan)
        
        main_layout.addLayout(button_layout)

class MasukkanPasswordDialog(QDialog):
    password_entered = pyqtSignal(str) 
    
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setWindowTitle("Konfirmasi Transaksi")
        self.setFixedSize(350, 250)
        self.setWindowFlags(self.windowFlags() & ~Qt.WindowType.WindowContextHelpButtonHint)
        self.initUI()

    def initUI(self):
        self.setStyleSheet("""
            QDialog { background-color: white; border-radius: 15px; }
            QLabel { color: black; font-size: 14px; }
            #HeaderTitle { font-size: 16px; font-weight: bold; color: #5873E8; }
            QLineEdit { padding: 8px 10px; border: 1px solid #C0C0C0; border-radius: 8px; font-size: 14px; color: black; }
            QPushButton { border-radius: 8px; font-weight: bold; height: 35px; }
        """)

        main_layout = QVBoxLayout(self)
        main_layout.setContentsMargins(25, 20, 25, 20)
        main_layout.setSpacing(20)

        title = QLabel("Konfirmasi Transaksi")
        title.setObjectName("HeaderTitle")
        title.setAlignment(Qt.AlignmentFlag.AlignCenter)
        main_layout.addWidget(title)
        
        password_label = QLabel("Masukkan Password")
        main_layout.addWidget(password_label)

        self.password_input = QLineEdit()
        self.password_input.setPlaceholderText("Password")
        self.password_input.setEchoMode(QLineEdit.EchoMode.Password) 
        main_layout.addWidget(self.password_input)
        
        main_layout.addStretch()

        button_layout = QHBoxLayout()
        button_layout.setSpacing(10)
        
        btn_kembali = QPushButton("Kembali")
        btn_kembali.setStyleSheet("QPushButton { background-color: white; color: #5873E8; border: 1px solid #C0C0C0; } QPushButton:hover { background-color: #F0F2FF; }")
        btn_kembali.clicked.connect(self.reject)

        btn_lanjutkan = QPushButton("Lanjutkan")
        btn_lanjutkan.setStyleSheet("QPushButton { background-color: #5873E8; color: white; } QPushButton:hover { background-color: #4A63D0; }")
        btn_lanjutkan.clicked.connect(self.emit_password_and_accept) 
        
        button_layout.addWidget(btn_kembali)
        button_layout.addWidget(btn_lanjutkan)
        
        main_layout.addLayout(button_layout)
        
    def emit_password_and_accept(self):
        self.password_entered.emit(self.password_input.text())
        self.accept()

class PasswordSalahDialog(QDialog):
    return_to_form = pyqtSignal()
    try_again = pyqtSignal() 
    
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setWindowTitle("Password Salah")
        self.setFixedSize(380, 250) 
        self.setWindowFlags(self.windowFlags() & ~Qt.WindowType.WindowContextHelpButtonHint) 
        self.initUI()

    def initUI(self):
        self.setStyleSheet("""
            QDialog { background-color: white; border-radius: 15px; }
            QLabel { color: black; font-size: 14px; }
            #ErrorTitle { font-size: 16px; font-weight: bold; color: #E85858; }
            #ErrorMessage { color: black; font-weight: normal; }
            QPushButton { border-radius: 8px; font-weight: bold; height: 35px; }
        """)

        main_layout = QVBoxLayout(self)
        main_layout.setContentsMargins(25, 20, 25, 20)
        main_layout.setSpacing(20)
        
        title = QLabel("Gagal Konfirmasi")
        title.setObjectName("ErrorTitle")
        title.setAlignment(Qt.AlignmentFlag.AlignCenter)
        main_layout.addWidget(title)
        
        message = QLabel("Password yang Anda masukkan salah.")
        message.setObjectName("ErrorMessage") 
        message.setAlignment(Qt.AlignmentFlag.AlignCenter)
        main_layout.addWidget(message)
        
        main_layout.addStretch()

        button_layout = QHBoxLayout()
        button_layout.setSpacing(10)
        
        btn_return = QPushButton("Kembali")
        btn_return.setStyleSheet("QPushButton { background-color: white; color: #5873E8; border: 1px solid #C0C0C0; } QPushButton:hover { background-color: #F0F2FF; }")
        btn_return.clicked.connect(self.return_to_form.emit) 
        btn_return.clicked.connect(self.close)

        btn_try_again = QPushButton("Coba Lagi")
        btn_try_again.setStyleSheet("QPushButton { background-color: #5873E8; color: white; } QPushButton:hover { background-color: #4A63D0; }")
        btn_try_again.clicked.connect(self.try_again.emit) 
        btn_try_again.clicked.connect(self.close)
        
        button_layout.addWidget(btn_return)
        button_layout.addWidget(btn_try_again)
        
        main_layout.addLayout(button_layout)

class SelesaiDialog(QDialog):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setFixedSize(350, 200) 
        self.setWindowFlags(self.windowFlags() & ~Qt.WindowType.WindowContextHelpButtonHint) 
        self.initUI()

    def initUI(self):
        self.setStyleSheet("""
            /* Pastikan QDialog punya warna background biru penuh */
            QDialog { 
                background-color: #5873E8; 
                border-radius: 15px; 
            } 
            QLabel { 
                color: white; 
                font-size: 14px; 
            }
            #SuccessText { 
                font-size: 16px; 
                font-weight: bold; 
            }
            /* Menghapus background default QLabel (putih/transparent) untuk menghindari pemotongan warna biru */
            QLabel {
                background-color: transparent; 
            }
        """)

        main_layout = QVBoxLayout(self)
        # Hapus semua margin pada layout utama untuk mengisi QDialog sepenuhnya
        main_layout.setContentsMargins(0, 0, 0, 0) 
        main_layout.setSpacing(10) 

        # --- Bagian 1: Spacer (Push content ke tengah) ---
        main_layout.addStretch() 
        
        # --- Bagian 2: Konten (Ikon dan Teks) ---
        
        icon_label = QLabel()
        # Menggunakan placeholder untuk ikon
        # Seharusnya icon = QIcon("img/Checkmark.svg") 
        icon_label.setText("✓") 
        icon_label.setStyleSheet("font-size: 40px; font-weight: bold;")
        icon_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        main_layout.addWidget(icon_label)
        
        message = QLabel("Berhasil Menambahkan Rekening")
        message.setObjectName("SuccessText")
        message.setAlignment(Qt.AlignmentFlag.AlignCenter)
        main_layout.addWidget(message)
        
        # --- Bagian 3: Spacer (Push content ke tengah) ---
        main_layout.addStretch() 


# --- 2. Main View: Tambah Rekening ---
class TambahRekeningView(QWidget):
    
    rekening_added = pyqtSignal()  # Signal ketika rekening berhasil ditambahkan
    
    def __init__(self, user, account_controller, parent=None):
        super().__init__(parent)
        self.user = user
        self.account_controller = account_controller
        
        # Edit mode properties
        self.edit_mode = False
        self.edit_account_id = None
        self.edit_account_data = None
        
        self.initUI()
        
    def initUI(self):
        self.setWindowTitle("Tabungin - Tambah Rekening")
        # self.setFixedSize(800, 600)  <-- Removed to allow responsive layout
        self.setStyleSheet("background-color: white;")

        main_layout = QHBoxLayout()
        main_layout.setContentsMargins(0, 0, 0, 0)
        self.setLayout(main_layout)

        # Sidebar is handled by MainWindow, so we don't need to create it here
        # But if this is a standalone view/dialog, we might need it.
        # Assuming this is part of the main stack, we just show the content.
        
        content_widget = QWidget()
        content_layout = QVBoxLayout(content_widget)
        content_layout.setContentsMargins(50, 40, 50, 40)
        content_layout.setSpacing(25)
        
        # Header
        header_layout = QHBoxLayout()
        
        # Tombol Kembali (Arrow Left) - Optional if handled by sidebar
        # self.btn_back = QPushButton()
        # self.btn_back.setFixedSize(32, 32)
        # icon_back = QIcon("img/ArrowLeft.svg") 
        # if icon_back.isNull():
        #      self.btn_back.setText("<")
        #      self.btn_back.setStyleSheet("QPushButton { font-size: 20px; border: none; }")
        # else:
        #     self.btn_back.setIcon(icon_back)
        #     self.btn_back.setIconSize(QSize(24, 24))
        #     self.btn_back.setStyleSheet("""
        #         QPushButton { background-color: transparent; border: none; border-radius: 5px; }
        #         QPushButton:hover { background-color: #F0F2FF; }
        #     """)
        # header_layout.addWidget(self.btn_back)
        
        self.title_label = QLabel("Tambah Rekening")
        self.title_label.setStyleSheet("font-size: 20px; font-weight: bold; color: black;")
        
        # header_layout.addSpacing(15) 
        header_layout.addWidget(self.title_label)
        header_layout.addStretch()
        content_layout.addLayout(header_layout)

        content_layout.addSpacerItem(QSpacerItem(20, 10, QSizePolicy.Policy.Minimum, QSizePolicy.Policy.Fixed))
        
        self.createForm(content_layout)
        
        content_layout.addStretch()
        
        main_layout.addWidget(content_widget, 1)

    def createForm(self, layout):
        # 1. Jenis E-wallet
        label_ewallet = QLabel("Jenis E-wallet")
        label_ewallet.setStyleSheet("font-size: 14px; font-weight: bold; color: black;")
        layout.addWidget(label_ewallet)
        
        self.combo_ewallet = QComboBox()
        self.combo_ewallet.setFixedHeight(45)
        self.combo_ewallet.setEditable(False)
        self.combo_ewallet.addItems(["GOPAY", "DANA", "SHOPEEPAY", "OVO", "LINKAJA"])
        self.combo_ewallet.setPlaceholderText("Pilih jenis e-wallet")
        self.combo_ewallet.setStyleSheet("""
            QComboBox {
                border: 1px solid #C0C0C0;
                border-radius: 8px;
                padding: 5px 10px;
                font-size: 14px;
                color: black;
                background-color: white;
            }
            QComboBox::drop-down {
                border: none;
            }
            QComboBox::down-arrow {
                image: url(img/ChevronDown.svg); 
                width: 12px;
                height: 12px;
                margin-right: 15px;
            }
            QComboBox QAbstractItemView {
                background-color: white;
                color: black;
                selection-background-color: #5873E8;
                selection-color: white;
                border: 1px solid #C0C0C0;
            }
        """)
        layout.addWidget(self.combo_ewallet)
        
        # 2. Nomor Rekening
        label_norek = QLabel("Nomor Rekening")
        label_norek.setStyleSheet("font-size: 14px; font-weight: bold; color: black;")
        layout.addWidget(label_norek)
        
        self.input_norek = QLineEdit()
        self.input_norek.setPlaceholderText("Masukkan nomor rekening")
        self.input_norek.setFixedHeight(45)
        self.input_norek.setValidator(QIntValidator(0, 999999999))
        self.input_norek.setStyleSheet("""
            QLineEdit {
                border: 1px solid #C0C0C0;
                border-radius: 8px;
                padding: 5px 10px;
                font-size: 14px;
                color: black;
            }
        """)
        layout.addWidget(self.input_norek)
        
        # 3. Nominal Awal
        label_nominal = QLabel("Nominal Awal")
        label_nominal.setStyleSheet("font-size: 14px; font-weight: bold; color: black;")
        layout.addWidget(label_nominal)
        
        self.input_nominal = QLineEdit()
        self.input_nominal.setPlaceholderText("Masukkan nominal awal")
        self.input_nominal.setFixedHeight(45)
        self.input_nominal.setValidator(QIntValidator(0, 999999999))
        self.input_nominal.setStyleSheet("""
            QLineEdit {
                border: 1px solid #C0C0C0;
                border-radius: 8px;
                padding: 5px 10px;
                font-size: 14px;
                color: black;
            }
        """)
        layout.addWidget(self.input_nominal)
        
        # 4. Deskripsi (Optional - mapped to account name for now)
        label_deskripsi = QLabel("Deskripsi (Nama Rekening)")
        label_deskripsi.setStyleSheet("font-size: 14px; font-weight: bold; color: black;")
        layout.addWidget(label_deskripsi)
        
        self.input_deskripsi = QLineEdit()
        self.input_deskripsi.setPlaceholderText("Contoh: Tabungan Liburan")
        self.input_deskripsi.setFixedHeight(45)
        self.input_deskripsi.setStyleSheet("""
            QLineEdit {
                border: 1px solid #C0C0C0;
                border-radius: 8px;
                padding: 5px 10px;
                font-size: 14px;
                color: black;
            }
        """)
        layout.addWidget(self.input_deskripsi)
        
        layout.addSpacing(20)
        
        # Tombol Konfirmasi
        self.btn_konfirmasi = QPushButton("Konfirmasi")
        self.btn_konfirmasi.setFixedHeight(50)
        self.btn_konfirmasi.setCursor(Qt.CursorShape.PointingHandCursor)
        self.btn_konfirmasi.setStyleSheet("""
            QPushButton {
                background-color: #5873E8;
                color: white;
                border-radius: 8px;
                font-size: 16px;
                font-weight: bold;
            }
            QPushButton:hover {
                background-color: #4A63D0;
            }
        """)
        self.btn_konfirmasi.clicked.connect(self.onKonfirmasiClicked)
        layout.addWidget(self.btn_konfirmasi)

    # --- Step Flow Handlers ---
    def onKonfirmasiClicked(self):
        ewallet_text = self.combo_ewallet.currentText().strip()
        
        if not ewallet_text:
            msg = QMessageBox(self)
            msg.setWindowTitle("Peringatan Input")
            msg.setText("Anda **Belum memilih rekening** atau mengetik nama e-wallet.")
            msg.setIcon(QMessageBox.Icon.Warning)
            msg.setStyleSheet("""
                QMessageBox { background-color: white; } 
                QLabel { color: black; }
                QPushButton { background-color: #5873E8; color: white; border-radius: 5px; padding: 5px 15px; }
            """)
            msg.exec()
            return
        
        self.current_ewallet = ewallet_text
        self.current_norek = self.input_norek.text() if self.input_norek.text() else "TIDAK DIISI"
        nominal_str = self.input_nominal.text()
        self.current_nominal = int(nominal_str) if nominal_str.isdigit() else 0 
        self.current_deskripsi = self.input_deskripsi.text() if self.input_deskripsi.text() else "TIDAK DIISI"

        conf_dialog = KonfirmasiRekeningDialog(self.current_ewallet, self.current_norek, self.current_nominal, self.current_deskripsi, self)
        self._center_dialog(conf_dialog)

        if conf_dialog.exec() == QDialog.DialogCode.Accepted: 
            self.onKonfirmasiLanjut()

    def onKonfirmasiLanjut(self):
        self.password_dialog = MasukkanPasswordDialog(self)
        self.password_dialog.password_entered.connect(self.checkPassword)
        self._center_dialog(self.password_dialog)
        self.password_dialog.exec()

    def checkPassword(self, password):
        # Verify password using user object
        if self.user and self.user.verify_password(password):
            self.onPasswordConfirmed()
        else:
            self.onPasswordFailed()
            
    def onPasswordFailed(self):
        error_dialog = PasswordSalahDialog(self)
        error_dialog.return_to_form.connect(self.resetUI) 
        error_dialog.try_again.connect(self.onKonfirmasiLanjut) 
        self._center_dialog(error_dialog)
        error_dialog.exec()

    def onPasswordConfirmed(self):
        # Save to database
        if self.account_controller and self.user:
            if self.edit_mode:
                # Update existing account (only name/type, not balance)
                display_name = f"{self.current_deskripsi} ({self.current_ewallet})"
                
                success = self.account_controller.update_account(
                    account_id=self.edit_account_id,
                    account_name=display_name
                    # Note: balance is not updated - it should only change via transactions
                )
                
                if success:
                    selesai_dialog = SelesaiDialog(self)
                    self._center_dialog(selesai_dialog)
                    selesai_dialog.show()
                    
                    # Emit signal and close/reset after delay
                    QTimer.singleShot(1500, selesai_dialog.close) 
                    QTimer.singleShot(1500, self.rekening_added.emit)
                    QTimer.singleShot(2000, self.resetUI)
                else:
                    QMessageBox.critical(self, "Error", "Gagal memperbarui rekening.")
            else:
                # Create new account
                # Use account number as ID, or generate one if empty/duplicate handling needed
                # For now using logic from previous TambahRekening.py
                account_id = self.current_norek
                
                # Combine name for display: "Deskripsi (EWALLET)"
                display_name = f"{self.current_deskripsi} ({self.current_ewallet})"
                
                success = self.account_controller.create_account(
                    account_id=account_id,
                    account_name=display_name,
                    user_id=self.user.user_id,
                    initial_balance=self.current_nominal
                )
                
                if success:
                    selesai_dialog = SelesaiDialog(self)
                    self._center_dialog(selesai_dialog)
                    selesai_dialog.show()
                    
                    # Emit signal and close/reset after delay
                    QTimer.singleShot(1500, selesai_dialog.close) 
                    QTimer.singleShot(1500, self.rekening_added.emit)
                    QTimer.singleShot(2000, self.resetUI)
                else:
                    error_msg = QMessageBox(self)
                    error_msg.setWindowTitle("Error")
                    error_msg.setText("Error: tidak bisa membuat rekening dengan nomor rekening yang sudah terdaftar")
                    error_msg.setIcon(QMessageBox.Icon.Critical)
                    error_msg.setStyleSheet("""
                        QMessageBox { background-color: white; }
                        QLabel { color: black; font-size: 14px; font-weight: bold; }
                        QPushButton { 
                            background-color: #E85858; 
                            color: white; 
                            border-radius: 5px; 
                            padding: 5px 15px;
                            font-size: 14px;
                            font-weight: bold;
                        }
                        QPushButton:hover { background-color: #D44545; }
                    """)
                    error_msg.exec()
        else:
            QMessageBox.critical(self, "Error", "Controller error.")

    def _center_dialog(self, dialog):
        # Menghitung posisi tengah relatif terhadap parent (self/TambahRekeningView)
        parent_rect = self.geometry()
        x = parent_rect.x() + (parent_rect.width() - dialog.width()) // 2
        y = parent_rect.y() + (parent_rect.height() - dialog.height()) // 2
        dialog.move(x, y)

    def resetUI(self): 
        self.combo_ewallet.setEditText("")
        self.input_norek.clear()
        self.input_nominal.clear()
        self.input_deskripsi.clear()
        self.clear_edit_mode()

    def set_edit_mode(self, account_id: str, account_data: dict):
        """Set the form to edit mode with existing account data"""
        self.edit_mode = True
        self.edit_account_id = account_id
        self.edit_account_data = account_data
        
        # Update title
        self.title_label.setText("Edit Rekening")
        
        # Populate form with existing data
        ewallet = account_data.get('ewallet', '')
        norek = account_data.get('norek', '')
        nominal = account_data.get('nominal', 0)
        deskripsi = account_data.get('deskripsi', '')
        
        # Set ewallet combo
        index = self.combo_ewallet.findText(ewallet)
        if index >= 0:
            self.combo_ewallet.setCurrentIndex(index)
        else:
            # If ewallet not in list, set to first item
            self.combo_ewallet.setCurrentIndex(0)
        
        # Set other fields
        self.input_norek.setText(norek)
        self.input_norek.setReadOnly(True)  # Prevent changing account ID
        self.input_norek.setStyleSheet("""
            QLineEdit {
                border: 1px solid #C0C0C0;
                border-radius: 8px;
                padding: 5px 10px;
                font-size: 14px;
                color: #888888;
                background-color: #f0f0f0;
            }
        """)
        self.input_nominal.setText(str(int(nominal)))
        self.input_nominal.setReadOnly(True)  # Prevent changing balance directly
        self.input_nominal.setStyleSheet("""
            QLineEdit {
                border: 1px solid #C0C0C0;
                border-radius: 8px;
                padding: 5px 10px;
                font-size: 14px;
                color: #888888;
                background-color: #f0f0f0;
            }
        """)
        self.input_deskripsi.setText(deskripsi)
        
        # Update button text
        self.btn_konfirmasi.setText("Simpan Perubahan")
    
    def clear_edit_mode(self):
        """Clear edit mode and reset to add mode"""
        self.edit_mode = False
        self.edit_account_id = None
        self.edit_account_data = None
        
        # Reset title
        self.title_label.setText("Tambah Rekening")
        
        # Reset button text
        self.btn_konfirmasi.setText("Konfirmasi")
        
        # Reset norek field to editable
        self.input_norek.setReadOnly(False)
        self.input_norek.setStyleSheet("""
            QLineEdit {
                border: 1px solid #C0C0C0;
                border-radius: 8px;
                padding: 5px 10px;
                font-size: 14px;
                color: black;
            }
        """)
        
        # Reset nominal field to editable
        self.input_nominal.setReadOnly(False)
        self.input_nominal.setStyleSheet("""
            QLineEdit {
                border: 1px solid #C0C0C0;
                border-radius: 8px;
                padding: 5px 10px;
                font-size: 14px;
                color: black;
            }
        """)

