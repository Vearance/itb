import sys
import uuid
from PyQt6.QtWidgets import (
    QWidget, QLabel, QLineEdit, QPushButton, QVBoxLayout, QHBoxLayout, 
    QFrame, QMessageBox, QStackedWidget, QSizePolicy, QGraphicsDropShadowEffect,
    QGridLayout, QComboBox, QSpacerItem
)
from PyQt6.QtCore import Qt, pyqtSignal, QSize
from PyQt6.QtGui import QFont, QColor, QPalette, QIntValidator

from controllers.LoginController import LoginController

class LoginView(QWidget):
    login_success = pyqtSignal(object)  # Signal to emit User object on successful login
    
    def __init__(self, login_controller: LoginController, account_controller=None, 
                 transaction_controller=None, target_controller=None, report_controller=None):
        super().__init__()
        self.controller = login_controller
        self.account_controller = account_controller
        self.transaction_controller = transaction_controller
        self.target_controller = target_controller
        self.report_controller = report_controller
        self.homepage = None
        self.current_user = None  # Store logged in user
        self.showLoginPage()

    def showLoginPage(self):
        self.setWindowTitle("Tabungin - Login")
        self.setFixedSize(1000, 700)
        
        main_layout = QHBoxLayout()
        main_layout.setContentsMargins(0, 0, 0, 0)
        main_layout.setSpacing(0)
        self.setLayout(main_layout)

        left_frame = QFrame()
        left_frame.setObjectName("leftSideFrame") 
        
        left_frame.setStyleSheet("""
            QFrame#leftSideFrame {
                background: qlineargradient(
                    spread:pad, x1:0, y1:0, x2:1, y2:1,
                    stop:0 #5873E8, 
                    stop:0.6 #5873E8,
                    stop:1 #7A8DF0
                );
            }
        """)
        
        left_layout = QVBoxLayout()
        left_layout.setContentsMargins(60, 50, 30, 50)
        left_frame.setLayout(left_layout)

        dots_container = QWidget()
        dots_grid = QGridLayout(dots_container)
        dots_grid.setSpacing(12)
        dots_grid.setContentsMargins(0, 0, 0, 0)

        for row in range(5):
            for col in range(5):
                dot = QFrame()
                dot.setFixedSize(6, 6)
                dot.setStyleSheet("background-color: rgba(255, 255, 255, 0.6); border-radius: 3px;")
                dots_grid.addWidget(dot, row, col)

        top_header_layout = QHBoxLayout()
        top_header_layout.addStretch()
        top_header_layout.addWidget(dots_container)

        welcome_html_text = """
            <html>
                <head/>
                <body>
                    <p style='line-height:0.95;'> 
                        <span style='font-weight:300;'>Hello,</span><br/>
                        <span style='font-weight:700;'>Welcome to<br/>Tabungin!</span>
                    </p>
                </body>
            </html>
        """
        
        welcome_label = QLabel(welcome_html_text)
        
        welcome_label.setStyleSheet("""
            QLabel {
                color: white; 
                font-size: 64px;
            }
        """)
        welcome_label.setAlignment(Qt.AlignmentFlag.AlignLeft | Qt.AlignmentFlag.AlignVCenter)
        welcome_label.setTextFormat(Qt.TextFormat.RichText)
        welcome_label.setWordWrap(True)
        welcome_label.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Preferred)

        left_layout.addLayout(top_header_layout)
        left_layout.addStretch(1)
        left_layout.addWidget(welcome_label)
        left_layout.addStretch(1)
        self.right_stack = QStackedWidget()
        self.right_stack.setStyleSheet("background-color: white;")
        
        self.login_widget = self.showLoginForm()
        self.register_widget = self.showRegisterForm()
        self.tambah_rekening_widget = self.showTambahRekeningForm()
        
        self.right_stack.addWidget(self.login_widget)
        self.right_stack.addWidget(self.register_widget)
        self.right_stack.addWidget(self.tambah_rekening_widget)

        main_layout.addWidget(left_frame, 1)
        main_layout.addWidget(self.right_stack, 1)

    def createErrorBanner(self):
        banner = QFrame()
        banner.setStyleSheet("""
            QFrame {
                background-color: #FFF4E5;
                border-radius: 5px;
                padding: 10px;
            }
        """)

        banner.hide()

        layout = QHBoxLayout(banner)
        layout.setContentsMargins(10, 5, 10, 5)
        layout.setSpacing(10)

        icon_label = QLabel("⚠️")
        icon_label.setStyleSheet("color: #FFB020; font-size: 20px;")
        layout.addWidget(icon_label)
        error_label = QLabel()
        error_label.setStyleSheet("color: #663C00; font-size: 12px;")
        error_label.setWordWrap(True)
        layout.addWidget(error_label, 1)

        return banner, error_label

    def createSuccessBanner(self):
        banner = QFrame()
        banner.setObjectName("successBanner")
        banner.setStyleSheet("""
            QFrame#successBanner {
                background-color: #E8F5E9;
                border: 1px solid #4CAF50;
                border-radius: 5px;
                padding: 10px;
            }
        """)

        banner.hide()

        layout = QHBoxLayout(banner)
        layout.setContentsMargins(10, 5, 10, 5)
        layout.setSpacing(10)

        icon_label = QLabel("✓")
        icon_label.setStyleSheet("color: #4CAF50; font-size: 20px; font-weight: bold; background-color: transparent;")
        layout.addWidget(icon_label)
        success_label = QLabel()
        success_label.setStyleSheet("color: #2E7D32; font-size: 12px; background-color: transparent;")
        success_label.setWordWrap(True)
        layout.addWidget(success_label, 1)

        return banner, success_label

    def showLoginForm(self):
        widget = QWidget()
        layout = QVBoxLayout()
        layout.setAlignment(Qt.AlignmentFlag.AlignCenter)
        layout.setContentsMargins(60, 60, 60, 60)
        layout.setSpacing(15)
        widget.setLayout(layout)

        title = QLabel("Login")
        title.setStyleSheet("color: #5873E8; font-weight: bold; font-size: 30px;")
        title.setAlignment(Qt.AlignmentFlag.AlignCenter)
        
        self.login_error_banner, self.login_error_text = self.createErrorBanner()
        self.login_success_banner, self.login_success_text = self.createSuccessBanner()
        
        self.username_input = QLineEdit()
        self.username_input.setPlaceholderText("Username")
        self.username_input.setStyleSheet(self.getInputStyle())
        self.username_input.setFixedHeight(50)

        self.password_input = QLineEdit()
        self.password_input.setPlaceholderText("Password")
        self.password_input.setEchoMode(QLineEdit.EchoMode.Password)
        self.password_input.setStyleSheet(self.getInputStyle())
        self.password_input.setFixedHeight(50)

        forgot_pass = QLabel("Forgot password?")
        forgot_pass.setStyleSheet("color: #5873E8; font-size: 12px;")
        forgot_pass.setCursor(Qt.CursorShape.PointingHandCursor)

        login_btn = QPushButton("Login")
        login_btn.setStyleSheet(self.getButtonStyle(primary=True))
        login_btn.setFixedHeight(50)
        login_btn.setCursor(Qt.CursorShape.PointingHandCursor)
        login_btn.clicked.connect(self.handle_login)

        divider_layout = QHBoxLayout()
        divider_layout.setSpacing(15)
        line1 = QFrame()
        line1.setFrameShape(QFrame.Shape.HLine)
        line1.setFrameShadow(QFrame.Shadow.Sunken)
        line1.setStyleSheet("background-color: #E0E0E0; max-height: 1px; border: none;")
        line1.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Fixed)
        
        line2 = QFrame()
        line2.setFrameShape(QFrame.Shape.HLine)
        line2.setFrameShadow(QFrame.Shadow.Sunken)
        line2.setStyleSheet("background-color: #E0E0E0; max-height: 1px; border: none;")
        line2.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Fixed)
        
        or_label = QLabel("Or")
        or_label.setStyleSheet("color: #5873E8; font-size: 14px; font-weight: bold;")
        or_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        
        divider_layout.addWidget(line1, 1)
        divider_layout.addWidget(or_label)
        divider_layout.addWidget(line2, 1)

        register_btn = QPushButton("Register")
        register_btn.setStyleSheet(self.getButtonStyle(primary=False))
        register_btn.setFixedHeight(50)
        register_btn.setCursor(Qt.CursorShape.PointingHandCursor)
        register_btn.clicked.connect(self.toggle_view)

        signup_layout = QHBoxLayout()
        signup_layout.setAlignment(Qt.AlignmentFlag.AlignCenter)
        signup_layout.setSpacing(5)
        no_account = QLabel("Belum punya akun?")
        no_account.setStyleSheet("color: grey; font-size: 12px;")
        signup_link = QPushButton("Sign up.")
        signup_link.setStyleSheet("border: none; color: #5873E8; font-weight: bold; text-align: left; text-decoration: underline;")
        signup_link.setCursor(Qt.CursorShape.PointingHandCursor)
        signup_link.clicked.connect(self.toggle_view)
        
        signup_layout.addWidget(no_account)
        signup_layout.addWidget(signup_link)

        layout.addWidget(title)
        layout.addSpacing(10)
        layout.addWidget(self.login_success_banner)
        layout.addWidget(self.login_error_banner)
        layout.addSpacing(10)
        layout.addWidget(self.username_input)
        layout.addWidget(self.password_input)
        layout.addWidget(forgot_pass)
        layout.addSpacing(10)
        layout.addWidget(login_btn)
        layout.addLayout(divider_layout)
        layout.addWidget(register_btn)
        layout.addSpacing(30)
        layout.addLayout(signup_layout)
        return widget

    def showRegisterForm(self):
        widget = QWidget()
        layout = QVBoxLayout()
        layout.setAlignment(Qt.AlignmentFlag.AlignCenter)
        layout.setContentsMargins(60, 60, 60, 60)
        layout.setSpacing(15)
        widget.setLayout(layout)

        title = QLabel("Register")
        title.setStyleSheet("color: #5873E8; font-weight: bold; font-size: 30px;")
        title.setAlignment(Qt.AlignmentFlag.AlignCenter)
        
        self.register_error_banner, self.register_error_text = self.createErrorBanner()
        
        self.reg_username_input = QLineEdit()
        self.reg_username_input.setPlaceholderText("Username")
        self.reg_username_input.setStyleSheet(self.getInputStyle())
        self.reg_username_input.setFixedHeight(50)

        self.reg_password_input = QLineEdit()
        self.reg_password_input.setPlaceholderText("Password")
        self.reg_password_input.setEchoMode(QLineEdit.EchoMode.Password)
        self.reg_password_input.setStyleSheet(self.getInputStyle())
        self.reg_password_input.setFixedHeight(50)

        self.reg_verify_input = QLineEdit()
        self.reg_verify_input.setPlaceholderText("Verify Password")
        self.reg_verify_input.setEchoMode(QLineEdit.EchoMode.Password)
        self.reg_verify_input.setStyleSheet(self.getInputStyle())
        self.reg_verify_input.setFixedHeight(50)

        register_btn = QPushButton("Register")
        register_btn.setStyleSheet(self.getButtonStyle(primary=True))
        register_btn.setFixedHeight(50)
        register_btn.setCursor(Qt.CursorShape.PointingHandCursor)
        register_btn.clicked.connect(self.handle_register)

        login_layout = QHBoxLayout()
        login_layout.setAlignment(Qt.AlignmentFlag.AlignCenter)
        login_layout.setSpacing(5)
        have_account = QLabel("Sudah punya akun?")
        have_account.setStyleSheet("color: grey; font-size: 12px;")
        login_link = QPushButton("Login.")
        login_link.setStyleSheet("border: none; color: #5873E8; font-weight: bold; text-align: left; text-decoration: underline;")
        login_link.setCursor(Qt.CursorShape.PointingHandCursor)
        login_link.clicked.connect(self.toggle_view)
        
        login_layout.addWidget(have_account)
        login_layout.addWidget(login_link)

        layout.addWidget(title)
        layout.addSpacing(10)
        layout.addWidget(self.register_error_banner)
        layout.addSpacing(10)
        layout.addWidget(self.reg_username_input)
        layout.addWidget(self.reg_password_input)
        layout.addWidget(self.reg_verify_input)
        layout.addSpacing(10)
        layout.addWidget(register_btn)
        layout.addSpacing(10)
        layout.addLayout(login_layout)
        
        return widget

    def showTambahRekeningForm(self):
        """Form untuk menambahkan rekening pertama kali"""
        widget = QWidget()
        layout = QVBoxLayout()
        layout.setAlignment(Qt.AlignmentFlag.AlignCenter)
        layout.setContentsMargins(60, 60, 60, 60)
        layout.setSpacing(15)
        widget.setLayout(layout)

        title = QLabel("Tambah Rekening Pertama")
        title.setStyleSheet("color: #5873E8; font-weight: bold; font-size: 26px;")
        title.setAlignment(Qt.AlignmentFlag.AlignCenter)
        
        subtitle = QLabel("Silakan tambahkan rekening e-wallet Anda")
        subtitle.setStyleSheet("color: #808080; font-size: 13px;")
        subtitle.setAlignment(Qt.AlignmentFlag.AlignCenter)
        
        # E-wallet dropdown (jenis tabungan)
        ewallet_label = QLabel("Jenis E-wallet")
        ewallet_label.setStyleSheet("font-size: 13px; font-weight: bold; color: black;")
        
        self.tambah_combo_ewallet = QComboBox()
        self.tambah_combo_ewallet.setFixedHeight(50)
        self.tambah_combo_ewallet.setEditable(False)
        self.tambah_combo_ewallet.addItems(["GOPAY", "DANA", "SHOPEEPAY", "OVO", "LINKAJA"])
        self.tambah_combo_ewallet.setPlaceholderText("Pilih jenis e-wallet")
        self.tambah_combo_ewallet.setStyleSheet("""
            QComboBox {
                border: 1px solid #5873E8;
                border-radius: 5px;
                padding: 5px 10px;
                font-size: 14px;
                color: black;
                background-color: white;
            }
            QComboBox:focus {
                border: 2px solid #5873E8;
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

        # Nama rekening (custom name)
        nama_label = QLabel("Nama Rekening")
        nama_label.setStyleSheet("font-size: 13px; font-weight: bold; color: black;")
        
        self.tambah_input_nama = QLineEdit()
        self.tambah_input_nama.setPlaceholderText("Misal: Tabungan Liburan, Dompet Utama, dll")
        self.tambah_input_nama.setStyleSheet(self.getInputStyle())
        self.tambah_input_nama.setFixedHeight(50)

        # Nomor rekening (will be used as account_id)
        norek_label = QLabel("Nomor Rekening")
        norek_label.setStyleSheet("font-size: 13px; font-weight: bold; color: black;")
        
        self.tambah_input_norek = QLineEdit()
        self.tambah_input_norek.setPlaceholderText("Masukkan nomor rekening")
        self.tambah_input_norek.setValidator(QIntValidator(0, 999999999))
        self.tambah_input_norek.setStyleSheet(self.getInputStyle())
        self.tambah_input_norek.setFixedHeight(50)

        # Nominal awal
        nominal_label = QLabel("Nominal Awal")
        nominal_label.setStyleSheet("font-size: 13px; font-weight: bold; color: black;")
        
        self.tambah_input_nominal = QLineEdit()
        self.tambah_input_nominal.setPlaceholderText("Masukkan nominal (e.g., 200000)")
        self.tambah_input_nominal.setValidator(QIntValidator(0, 999999999))
        self.tambah_input_nominal.setStyleSheet(self.getInputStyle())
        self.tambah_input_nominal.setFixedHeight(50)

        # Tombol simpan
        save_btn = QPushButton("Simpan Rekening")
        save_btn.setStyleSheet(self.getButtonStyle(primary=True))
        save_btn.setFixedHeight(50)
        save_btn.setCursor(Qt.CursorShape.PointingHandCursor)
        save_btn.clicked.connect(self.handle_save_account)

        layout.addWidget(title)
        layout.addWidget(subtitle)
        layout.addSpacing(20)
        layout.addWidget(ewallet_label)
        layout.addWidget(self.tambah_combo_ewallet)
        layout.addWidget(nama_label)
        layout.addWidget(self.tambah_input_nama)
        layout.addWidget(norek_label)
        layout.addWidget(self.tambah_input_norek)
        layout.addWidget(nominal_label)
        layout.addWidget(self.tambah_input_nominal)
        layout.addSpacing(10)
        layout.addWidget(save_btn)
        
        return widget

    def getInputStyle(self):
        return """
            QLineEdit {
                border: 1px solid #5873E8;
                border-radius: 5px;
                padding: 5px 10px;
                font-size: 14px;
                color: black;
                background-color: white;
            }
            QLineEdit:focus {
                border: 2px solid #5873E8;
            }
        """

    def getInputErrorStyle(self):
        return """
            QLineEdit {
                border: 2px solid #FF6B6B;
                border-radius: 5px;
                padding: 5px 10px;
                font-size: 14px;
                color: black;
                background-color: white;
            }
            QLineEdit:focus {
                border: 2px solid #FF6B6B;
            }
        """

    def setInputError(self, *inputs):
        """Apply error styling to input fields"""
        for input_field in inputs:
            input_field.setStyleSheet(self.getInputErrorStyle())

    def clearInputError(self, *inputs):
        """Remove error styling from input fields"""
        for input_field in inputs:
            input_field.setStyleSheet(self.getInputStyle())

    def getButtonStyle(self, primary=True):
        if primary:
            return """
                QPushButton {
                    background-color: #5873E8;
                    color: white;
                    border: none;
                    border-radius: 5px;
                    font-size: 14px;
                    font-weight: bold;
                }
                QPushButton:hover {
                    background-color: #4a63d0;
                }
            """
        else:
            return """
                QPushButton {
                    background-color: transparent;
                    color: #5873E8;
                    border: 1px solid #5873E8;
                    border-radius: 5px;
                    font-size: 14px;
                    font-weight: bold;
                }
                QPushButton:hover {
                    background-color: #f0f2ff;
                }
            """

    def toggle_view(self):
        if self.right_stack.currentIndex() == 0:
            self.right_stack.setCurrentIndex(1)
        else:
            self.right_stack.setCurrentIndex(0)

    def showUsernameError(self):
        self.login_error_text.setText("Username tidak ditemukan!")
        self.login_error_banner.show()
        self.setInputError(self.username_input)

    def showPasswordError(self):
        self.login_error_text.setText("Password yang kamu masukkan salah!")
        self.login_error_banner.show()
        self.setInputError(self.password_input)

    def show_message(self, title, message, icon=QMessageBox.Icon.Information):
        if icon in [QMessageBox.Icon.Critical, QMessageBox.Icon.Warning]:
            if self.right_stack.currentIndex() == 0: # Login View
                self.login_error_text.setText(message)
                self.login_error_banner.show()
                self.setInputError(self.username_input, self.password_input)
            else:
                self.register_error_text.setText(message)
                self.register_error_banner.show()
                self.setInputError(self.reg_username_input, self.reg_password_input, self.reg_verify_input)
        else:
            msg = QMessageBox(self)
            msg.setWindowTitle(title)
            msg.setText(message)
            msg.setIcon(icon)
            msg.exec()

    def clear_errors(self):
        self.login_error_banner.hide()
        self.login_success_banner.hide()
        self.register_error_banner.hide()
        self.clearInputError(self.username_input, self.password_input)
        self.clearInputError(self.reg_username_input, self.reg_password_input, self.reg_verify_input)

    def handle_login(self):
        username = self.username_input.text()
        password = self.password_input.text()
        
        self.clear_errors()

        if not username or not password:
            self.show_message("Error", "Harap isi semua kolom", QMessageBox.Icon.Warning)
            return

        login_status = self.controller.login(username, password)

        if login_status == "SUCCESS":
            # Get the logged-in user
            user = self.controller.get_current_user()
            self.current_user = user
            
            # Check if user has accounts
            if self.account_controller:
                accounts = self.account_controller.get_user_accounts(user.user_id)
                if not accounts or len(accounts) == 0:
                    # No accounts, show TambahRekening form
                    print(f"User {username} has no accounts, showing account creation form")
                    self.right_stack.setCurrentIndex(2)  # Switch to TambahRekening
                    return
            
            # User has accounts, emit success signal
            self.login_success.emit(user)
            print(f"User {username} logged in successfully.")
        elif login_status == "USER_NOT_FOUND":
            self.showUsernameError()
        elif login_status == "WRONG_PASSWORD":
            self.showPasswordError()
        else:
            self.show_message("Error", "Terjadi kesalahan", QMessageBox.Icon.Critical)


    def handle_register(self):
        username = self.reg_username_input.text()
        password = self.reg_password_input.text()
        verify_password = self.reg_verify_input.text()
        
        self.clear_errors()

        if not username or not password or not verify_password:
            self.show_message("Error", "Harap isi semua kolom", QMessageBox.Icon.Warning)
            return

        if password != verify_password:
            self.show_message("Error", "Kata sandi tidak cocok", QMessageBox.Icon.Warning)
            return

        user_id = str(uuid.uuid4())
        success = self.controller.register_user(user_id, username, password)

        if success:
            self.toggle_view()
            self.login_success_text.setText("Registrasi Berhasil! Silakan Login.")
            self.login_success_banner.show()
        else:
            self.show_message("Error", "Registrasi Gagal. Username mungkin sudah ada.", QMessageBox.Icon.Critical)

    def handle_save_account(self):
        """Handle saving new account for user who just logged in"""
        ewallet_type = self.tambah_combo_ewallet.currentText().strip()
        account_name = self.tambah_input_nama.text().strip()
        account_number = self.tambah_input_norek.text().strip()
        nominal_str = self.tambah_input_nominal.text().strip()
        
        # Validation
        if not ewallet_type:
            QMessageBox.warning(self, "Error", "Jenis E-wallet tidak boleh kosong!")
            return
        
        if not account_name:
            QMessageBox.warning(self, "Error", "Nama rekening tidak boleh kosong!")
            return
        
        if not account_number:
            QMessageBox.warning(self, "Error", "Nomor rekening tidak boleh kosong!")
            return
        
        if not nominal_str or not nominal_str.isdigit():
            QMessageBox.warning(self, "Error", "Nominal harus berupa angka!")
            return
        
        nominal = float(nominal_str)
        
        # Use account_number as account_id (not UUID)
        account_id = account_number
        
        # Combine ewallet type with account name for display
        # Format: "Nama Rekening (JENIS_EWALLET)"
        display_name = f"{account_name} ({ewallet_type})"
        
        # Use AccountController to create account
        if self.account_controller and self.current_user:
            success = self.account_controller.create_account(
                account_id=account_id,
                account_name=display_name,  # Use combined name
                user_id=self.current_user.user_id,
                initial_balance=nominal
            )
            
            if success:
                QMessageBox.information(self, "Berhasil", 
                    f"Rekening '{account_name}' berhasil ditambahkan!\nJenis: {ewallet_type}")
                
                # Clear form
                self.tambah_combo_ewallet.setEditText("")
                self.tambah_input_nama.clear()
                self.tambah_input_norek.clear()
                self.tambah_input_nominal.clear()
                
                # Emit login success to proceed to main app
                self.login_success.emit(self.current_user)
            else:
                QMessageBox.critical(self, "Error", "Nomor rekening sudah ada! Silakan gunakan nomor rekening yang lain.")
        else:
            QMessageBox.critical(self, "Error", "Account controller tidak tersedia!")
