import sys
import uuid
from PyQt6.QtWidgets import (
    QWidget, QLabel, QPushButton, QVBoxLayout, QHBoxLayout,
    QFrame, QLineEdit, QComboBox, QDialog, QSizePolicy, QSpacerItem,
    QMessageBox, QScrollArea, QButtonGroup, QApplication 
)
from PyQt6.QtCore import Qt, pyqtSignal, QSize, QTimer
from PyQt6.QtGui import QIcon, QIntValidator, QFont

from models.Transaction import TransactionType 

# --- 1. Utility Classes (Dialogs) ---

class KonfirmasiTransaksiDialog(QDialog):
    """Dialog untuk menampilkan detail transaksi sebelum konfirmasi password."""
    def __init__(self, data, parent=None):
        super().__init__(parent)
        self.setWindowTitle("Konfirmasi Transaksi")
        self.setFixedSize(400, 380) # Ukuran diperbesar sedikit untuk tipe transaksi
        self.setWindowFlags(self.windowFlags() & ~Qt.WindowType.WindowContextHelpButtonHint) 
        self.initUI(data)
    
    def create_data_row(self, label_text, value_text):
        h_layout = QHBoxLayout()
        label = QLabel(label_text)
        value = QLabel(value_text)
        value.setObjectName("DataValue")
        value.setAlignment(Qt.AlignmentFlag.AlignRight)
        
        # Atur wrapping untuk deskripsi panjang
        if label_text == "Deskripsi":
            value.setWordWrap(True) 
        
        h_layout.addWidget(label)
        h_layout.addWidget(value, 1) # Berikan stretch agar value mengambil sisa ruang
        return h_layout

    def format_rupiah(self, nominal):
        """Helper untuk format nominal ke mata uang Rupiah."""
        try:
            return f"Rp{int(nominal):,}".replace(",", ".")
        except:
            return "Rp0"

    def initUI(self, data):
        self.setStyleSheet("""
            QDialog { background-color: white; border-radius: 15px; }
            QLabel { color: black; font-size: 14px; }
            #HeaderTitle { font-size: 18px; font-weight: bold; color: #5873E8; }
            #DataValue { font-weight: 500; }
            #TipeTransaksi { font-weight: bold; color: #28a745; } /* Warna hijau untuk tipe */
            QPushButton { border-radius: 8px; font-weight: bold; height: 35px; }
        """)

        main_layout = QVBoxLayout(self)
        main_layout.setContentsMargins(25, 20, 25, 20)
        main_layout.setSpacing(15)
        
        title = QLabel("Konfirmasi Transaksi")
        title.setObjectName("HeaderTitle")
        title.setAlignment(Qt.AlignmentFlag.AlignCenter)
        main_layout.addWidget(title)
        
        data_frame = QFrame()
        data_layout = QVBoxLayout(data_frame)
        data_layout.setContentsMargins(0, 10, 0, 10)
        data_layout.setSpacing(10)

        # BARU: Tampilkan Tipe Transaksi
        tipe_label_text = data.get("tipe_transaksi", "N/A")
        tipe_label = QLabel(f"Tipe: {tipe_label_text}")
        tipe_label.setObjectName("TipeTransaksi")
        if tipe_label_text == "Pengeluaran":
             tipe_label.setStyleSheet("font-weight: bold; color: #dc3545;") # Warna merah untuk Pengeluaran
        
        data_layout.addWidget(tipe_label)
        data_layout.addLayout(self.create_data_row("Rekening Sumber", data.get("sumber", "N/A")))
        data_layout.addLayout(self.create_data_row("Kategori Transaksi", data.get("tujuan", "N/A"))) # Update label
        data_layout.addLayout(self.create_data_row("Detail Tujuan/Sumber", data.get("no_tujuan", "N/A"))) # Update label
        
        # Nominal perlu diformat
        data_layout.addLayout(self.create_data_row("Nominal", self.format_rupiah(data.get("nominal", 0)))) 
        data_layout.addLayout(self.create_data_row("Deskripsi", data.get("deskripsi", "TIDAK DIISI")))

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
    """Dialog untuk memasukkan password konfirmasi."""
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
            QLineEdit { padding: 8px 10px; border: 1px solid #C0C0C0; border-radius: 8px; font-size: 14px; color: black; height: 45px; }
            QPushButton { border-radius: 8px; font-weight: bold; height: 35px; }
        """)

        main_layout = QVBoxLayout(self)
        main_layout.setContentsMargins(25, 20, 25, 20)
        main_layout.setSpacing(15)

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

class SaldoTidakCukupDialog(QDialog):
    """Dialog saat saldo tidak cukup untuk transaksi pengeluaran."""
    return_to_form = pyqtSignal()
    
    def __init__(self, saldo_tersedia: float, nominal_transaksi: float, nama_rekening: str, parent=None):
        super().__init__(parent)
        self.setWindowTitle("Jumlah Uang Tidak Mencukupi")
        self.setFixedSize(400, 320)
        self.setWindowFlags(self.windowFlags() & ~Qt.WindowType.WindowContextHelpButtonHint)
        self.saldo_tersedia = saldo_tersedia
        self.nominal_transaksi = nominal_transaksi
        self.nama_rekening = nama_rekening
        self.initUI()

    def initUI(self):
        self.setStyleSheet("""
            QDialog { background-color: white; border-radius: 15px; }
            QLabel { color: black; }
            #ErrorTitle { font-size: 16px; font-weight: bold; color: #dc3545; }
            #ErrorMessage { font-size: 13px; color: #555; }
            #DetailLabel { font-size: 12px; font-weight: 500; color: #333; }
            #DetailValue { font-size: 12px; color: #666; }
            QPushButton { border-radius: 8px; font-weight: bold; height: 36px; }
        """)

        main_layout = QVBoxLayout(self)
        main_layout.setContentsMargins(25, 20, 25, 20)
        main_layout.setSpacing(12)
        
        # Title
        title = QLabel("Jumlah Uang Tidak Mencukupi")
        title.setObjectName("ErrorTitle")
        title.setAlignment(Qt.AlignmentFlag.AlignCenter)
        main_layout.addWidget(title)
        
        # Main message
        message = QLabel("Saldo rekening tidak mencukupi untuk melakukan transaksi ini.")
        message.setObjectName("ErrorMessage") 
        message.setAlignment(Qt.AlignmentFlag.AlignCenter)
        message.setWordWrap(True)
        main_layout.addWidget(message)
        
        main_layout.addSpacing(5)
        
        # Detail info - tanpa frame, hanya teks
        detail_layout = QVBoxLayout()
        detail_layout.setContentsMargins(0, 0, 0, 0)
        detail_layout.setSpacing(8)
        
        # Rekening name
        rekening_label = QLabel(f"Rekening: <b>{self.nama_rekening}</b>")
        rekening_label.setObjectName("DetailLabel")
        rekening_label.setTextFormat(Qt.TextFormat.RichText)
        detail_layout.addWidget(rekening_label)
        
        # Saldo tersedia
        saldo_label = QLabel(f"Saldo Tersedia: <span style='color: #28a745; font-weight: bold;'>Rp{int(self.saldo_tersedia):,}</span>")
        saldo_label.setObjectName("DetailValue")
        saldo_label.setTextFormat(Qt.TextFormat.RichText)
        detail_layout.addWidget(saldo_label)
        
        # Nominal transaksi
        nominal_label = QLabel(f"Nominal Transaksi: <span style='color: #dc3545; font-weight: bold;'>Rp{int(self.nominal_transaksi):,}</span>")
        nominal_label.setObjectName("DetailValue")
        nominal_label.setTextFormat(Qt.TextFormat.RichText)
        detail_layout.addWidget(nominal_label)
        
        # Kekurangan
        kekurangan = self.nominal_transaksi - self.saldo_tersedia
        kekurangan_label = QLabel(f"Kekurangan: <span style='color: #dc3545; font-weight: bold;'>Rp{int(kekurangan):,}</span>")
        kekurangan_label.setObjectName("DetailValue")
        kekurangan_label.setTextFormat(Qt.TextFormat.RichText)
        detail_layout.addWidget(kekurangan_label)
        
        main_layout.addLayout(detail_layout)
        main_layout.addStretch()

        # Button
        button_layout = QHBoxLayout()
        button_layout.setSpacing(10)
        button_layout.setContentsMargins(0, 0, 0, 0)
        
        btn_kembali = QPushButton("Kembali")
        btn_kembali.setFixedWidth(120)
        btn_kembali.setStyleSheet("QPushButton { background-color: #5873E8; color: white; border: none; } QPushButton:hover { background-color: #4A63D0; }")
        btn_kembali.clicked.connect(self.return_to_form.emit) 
        btn_kembali.clicked.connect(self.close)
        
        button_layout.addStretch()
        button_layout.addWidget(btn_kembali)
        
        main_layout.addLayout(button_layout)

class PasswordSalahDialog(QDialog):
    """Dialog saat password salah, menawarkan coba lagi atau kembali."""
    return_to_form = pyqtSignal()
    try_again = pyqtSignal() 
    
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setWindowTitle("Gagal Konfirmasi")
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
        
        self.title_label = QLabel("Gagal Konfirmasi")
        self.title_label.setObjectName("ErrorTitle")
        self.title_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        main_layout.addWidget(self.title_label)
        
        self.message_label = QLabel("Password yang Anda masukkan salah. Mohon coba lagi.")
        self.message_label.setObjectName("ErrorMessage") 
        self.message_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.message_label.setWordWrap(True)
        main_layout.addWidget(self.message_label)
        
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
    """Dialog sukses yang akan hilang setelah beberapa saat."""
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setFixedSize(350, 200) 
        self.setWindowFlags(self.windowFlags() & ~Qt.WindowType.WindowContextHelpButtonHint) 
        self.initUI()

    def initUI(self):
        self.setStyleSheet("""
            QDialog { 
                background-color: #5873E8; 
                border-radius: 15px; 
            } 
            QLabel { 
                color: white; 
                font-size: 14px; 
                background-color: transparent; /* Penting */
            }
            #SuccessText { 
                font-size: 16px; 
                font-weight: bold; 
            }
        """)

        main_layout = QVBoxLayout(self)
        main_layout.setContentsMargins(0, 0, 0, 0) 
        main_layout.setSpacing(10) 

        main_layout.addStretch() 
        
        # Mencoba memuat ikon SVG atau fallback ke emoji
        icon_path = "img/Checkmark.svg"
        icon = QIcon(icon_path)
        
        if not icon.isNull():
             icon_label = QLabel()
             icon_label.setPixmap(icon.pixmap(QSize(64, 64)))
        else:
            icon_label = QLabel("✓") 
            icon_font = QFont()
            icon_font.setPointSize(48)
            icon_label.setFont(icon_font)

        icon_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        main_layout.addWidget(icon_label)
        
        message = QLabel("Transaksi Berhasil")
        message.setObjectName("SuccessText")
        message.setAlignment(Qt.AlignmentFlag.AlignCenter)
        main_layout.addWidget(message)
        
        main_layout.addStretch() 


# --- 2. Main View: Transaksi ---
class TransaksiView(QWidget):
    
    # Signals
    transaction_completed = pyqtSignal()  # Emitted when transaction is successfully added/updated
    back_to_previous = pyqtSignal()  # Signal untuk kembali ke halaman sebelumnya
    
    CORRECT_PASSWORD = "123"
    
    def __init__(self, user=None, account_controller=None, transaction_controller=None):
        super().__init__()
        self.user = user
        self.account_controller = account_controller
        self.transaction_controller = transaction_controller
        
        # Edit mode state
        self.edit_mode = False
        self.edit_transaction_id = None
        self.edit_transaction_data = None
        
        # State untuk menyimpan rekening sumber yang dipilih (sekarang menyimpan account_id)
        self.selected_account_id = None
        self.selected_account_name = None
        
        # State untuk tipe transaksi
        self.transaction_type = "Pengeluaran"  # Default: "Pengeluaran" atau "Pendapatan"
        
        # Cached accounts untuk mapping account_id <-> account_name
        self.user_accounts = []
        
        # Load user accounts from database if controllers are available
        self._load_user_accounts()
        
        self.initUI()
        self.setupConnections()
    
    def _load_user_accounts(self):
        """Load user accounts from database using account_controller."""
        if self.user and self.account_controller:
            try:
                self.user_accounts = self.account_controller.get_user_accounts(self.user.user_id)
                # Set default selected account
                if self.user_accounts:
                    self.selected_account_id = self.user_accounts[0].account_id
                    self.selected_account_name = self.user_accounts[0].account_name
            except Exception as e:
                print(f"Error loading user accounts: {e}")
                self.user_accounts = []
        
    def initUI(self):
        self.setWindowTitle("Tabungin - Transaksi")
        self.setStyleSheet("background-color: white;")
        self.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Expanding)

        main_layout = QHBoxLayout()
        main_layout.setContentsMargins(0, 0, 0, 0)
        self.setLayout(main_layout)
        
        # Konten utama (Transkasi)
        content_widget = QWidget()
        content_layout = QVBoxLayout(content_widget)
        # PENYESUAIAN SCROLLBAR: Margin kanan diperkecil menjadi 50 (sebelumnya 80)
        content_layout.setContentsMargins(50, 40, 50, 40) 
        
        # Header (Mengganti panah <- dengan QPushButton ikon)
        header_layout = QHBoxLayout()
        
        # Tombol Kembali (Arrow Left)
        self.btn_back = QPushButton()
        self.btn_back.setFixedSize(32, 32)
        # Menggunakan placeholder jika icon tidak tersedia
        icon_back = QIcon("img/ArrowLeft.svg") 
        if icon_back.isNull():
            self.btn_back.setText("<")
            self.btn_back.setStyleSheet("QPushButton { font-size: 20px; border: none; }")
        else:
            self.btn_back.setIcon(icon_back)
            self.btn_back.setIconSize(QSize(24, 24))
            self.btn_back.setStyleSheet("""
                QPushButton { background-color: transparent; border: none; border-radius: 5px; }
                QPushButton:hover { background-color: #F0F2FF; }
            """)
        header_layout.addWidget(self.btn_back)

        self.title_label = QLabel("Tambah Transaksi")
        # Style judul disesuaikan agar rapi berdampingan dengan tombol
        self.title_label.setStyleSheet("font-size: 20px; font-weight: bold; color: black; margin-left: 5px;") 
        
        header_layout.addWidget(self.title_label)
        header_layout.addStretch()
        content_layout.addLayout(header_layout)

        content_layout.addSpacerItem(QSpacerItem(20, 10, QSizePolicy.Policy.Minimum, QSizePolicy.Policy.Fixed))
        
        # Area Scrollable untuk Form
        scroll_area = QScrollArea()
        scroll_area.setWidgetResizable(True)
        scroll_area.setFrameShape(QFrame.Shape.NoFrame)
        scroll_area.setStyleSheet("background-color: white; border: none;")
        
        form_widget = QWidget()
        self.form_layout = QVBoxLayout(form_widget)
        self.form_layout.setContentsMargins(0, 0, 0, 0)
        self.form_layout.setSpacing(25)

        self.createForm(self.form_layout)
        
        scroll_area.setWidget(form_widget)
        content_layout.addWidget(scroll_area)
        
        # Tombol Konfirmasi (diluar area scroll agar selalu terlihat)
        self.btn_konfirmasi = QPushButton("Konfirmasi")
        self.btn_konfirmasi.setFixedHeight(50)
        self.btn_konfirmasi.setStyleSheet("""
            QPushButton { 
                background-color: #5873E8; color: white; border-radius: 8px; 
                font-size: 16px; font-weight: bold;
            }
            QPushButton:hover { background-color: #4A63D0; }
            QPushButton:disabled { background-color: #C0C0C0; }
        """)
        content_layout.addWidget(self.btn_konfirmasi)
        
        main_layout.addWidget(content_widget, 1)

    def createMinimalSidebar(self):
        """Membuat sidebar minimalis dengan ikon SVG, mengasumsikan file ada di folder 'img/'."""
        navMenu = QFrame()
        navMenu.setObjectName("navMenu")
        navMenu.setFixedWidth(110)
        navMenu.setSizePolicy(QSizePolicy.Policy.Fixed, QSizePolicy.Policy.Expanding)
        navMenu.setStyleSheet("background-color: white;")

        layout = QVBoxLayout(navMenu)
        layout.setContentsMargins(20, 40, 20, 40)
        layout.setSpacing(25)
        
        # Daftar item menu, menggunakan nama file SVG
        menu_items_map = [
            ("Home", "Home", False),
            ("Rekening", "TabunganSelected", True), # Aktif
            ("Cari", "Riwayat", False),
            ("Statistik", "Progress", False),
        ]
        
        # Warna monokromatik (abu-abu gelap) untuk ikon non-aktif
        MONOCHROME_COLOR = "#777777" 
        
        for label, icon_name, is_active in menu_items_map:
            btn = QPushButton()
            btn.setFixedSize(65, 65)
            
            # Path SVG yang diharapkan
            icon_path = f"img/{icon_name}.svg" 
            icon = QIcon(icon_path)
            
            # Jika icon berhasil dimuat, gunakan icon
            if not icon.isNull():
                btn.setIcon(icon)
                btn.setIconSize(QSize(35, 35))
                
                if is_active:
                    # Ikon aktif: latar belakang biru, ikon putih (asumsi SVG bisa di-color dengan CSS, atau SVG-nya memang putih)
                    btn.setStyleSheet("QPushButton { background-color: #5873E8; border-radius: 15px; }")
                else:
                    # Ikon non-aktif: Latar belakang transparan, ikon monokromatik abu-abu
                    btn.setStyleSheet(f"""
                        QPushButton {{ 
                            background-color: transparent; 
                            border: none; 
                            border-radius: 15px; 
                            /* Set fill color untuk SVG yang menggunakan 'currentColor' */
                            color: {MONOCHROME_COLOR}; 
                        }}
                        QPushButton:hover {{ background-color: #F0F2FF; }}
                    """)
            # Jika icon GAGAL dimuat (fallback ke emoji)
            else:
                if label == "Home": btn.setText("🏠")
                elif label == "Rekening": btn.setText("📦") 
                elif label == "Cari": btn.setText("⏱️")
                elif label == "Statistik": btn.setText("📈")
                
                # Style untuk fallback emoji
                if is_active:
                     btn.setStyleSheet("QPushButton { background-color: #5873E8; border-radius: 15px; color: white; font-size: 25px; }")
                else:
                    # Fallback non-aktif: monokromatik (emoji)
                    btn.setStyleSheet(f"""
                        QPushButton {{ background-color: transparent; border: none; border-radius: 15px; font-size: 25px; color: {MONOCHROME_COLOR}; }}
                        QPushButton:hover {{ background-color: #F0F2FF; }}
                    """)


            layout.addWidget(btn, 0, Qt.AlignmentFlag.AlignCenter)

        layout.addStretch()
        return navMenu

    def _get_type_button_style(self, is_checked, type_name):
        """Mengembalikan CSS untuk tombol Tipe Transaksi."""
        base_style = "border-radius: 8px; font-weight: bold; height: 40px;"
        
        if is_checked:
            # Warna untuk Pendapatan (hijau) atau Pengeluaran (merah)
            color = "#28a745" if type_name == "Pendapatan" else "#dc3545"
            return f"""
                QPushButton {{ 
                    background-color: {color}; 
                    color: white; 
                    border: none; 
                    {base_style}
                }} 
            """
        else:
            # Warna inactive
            color = "#28a745" if type_name == "Pendapatan" else "#dc3545"
            return f"""
                QPushButton {{ 
                    background-color: white; 
                    color: {color}; 
                    border: 1px solid {color}; 
                    {base_style}
                }} 
                QPushButton:hover {{ background-color: #f8f9fa; }}
            """

    def handleTypeSelection(self, button):
        """Memperbarui state dan style saat tombol tipe transaksi diklik."""
        if button.isChecked():
            self.transaction_type = button.text()
        
        # Memperbarui style semua tombol di grup
        for btn in self.type_button_group.buttons():
            btn.setStyleSheet(self._get_type_button_style(btn.isChecked(), btn.text()))

    def createForm(self, parent_layout: QVBoxLayout):
        """Membuat elemen-elemen input untuk Transaksi."""
        label_style = "font-size: 14px; font-weight: bold; color: black; margin-bottom: 5px;"
        input_style = """
            QLineEdit, QComboBox { padding: 8px 10px; border: 1px solid #C0C0C0; border-radius: 8px; font-size: 14px; color: black; height: 45px; }
            QPushButton { border-radius: 8px; font-weight: 500; height: 35px; }
        """
        dropdown_view_style = """
            QComboBox QAbstractItemView { 
                border: 1px solid #C0C0C0; 
                selection-background-color: #5873E8; 
                color: black; 
            }
        """
        
        # --- 0. Tipe Transaksi (Pendapatan/Pengeluaran) ---
        label_tipe = QLabel("Tipe Transaksi")
        label_tipe.setStyleSheet(label_style)
        parent_layout.addWidget(label_tipe)

        type_layout = QHBoxLayout()
        type_options = ["Pendapatan", "Pengeluaran"]
        self.type_button_group = QButtonGroup(self)
        self.type_button_group.setExclusive(True)

        for name in type_options:
            btn = QPushButton(name)
            btn.setCheckable(True)
            btn.setFixedSize(140, 40)
            
            if name == self.transaction_type:
                btn.setChecked(True)
                btn.setStyleSheet(self._get_type_button_style(True, name))
            else:
                btn.setStyleSheet(self._get_type_button_style(False, name))

            self.type_button_group.addButton(btn)
            type_layout.addWidget(btn)

        type_layout.addStretch()
        parent_layout.addLayout(type_layout)
        parent_layout.addSpacerItem(QSpacerItem(20, 10, QSizePolicy.Policy.Minimum, QSizePolicy.Policy.Fixed))


        # --- 1. Rekening Sumber (Horizontal Scrollable Buttons) ---
        label_sumber = QLabel("Rekening Sumber")
        label_sumber.setStyleSheet(label_style)
        parent_layout.addWidget(label_sumber)

        # Create QScrollArea for account selection buttons
        scroll_source = QScrollArea()
        scroll_source.setWidgetResizable(True)
        scroll_source.setFrameShape(QFrame.Shape.NoFrame)
        scroll_source.setHorizontalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAlwaysOn)
        scroll_source.setVerticalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAlwaysOff)
        scroll_source.setFixedHeight(60)
        
        # Store reference for later refresh
        self.source_scroll = scroll_source

        source_widget = QWidget()
        source_layout = QHBoxLayout(source_widget)
        source_layout.setContentsMargins(0, 0, 0, 0)

        # Use actual user accounts instead of hardcoded values
        self.source_button_group = QButtonGroup(self)
        self.source_button_group.setExclusive(True)

        for account in self.user_accounts:
            btn = QPushButton(account.account_name)
            btn.setCheckable(True)
            btn.setFixedSize(120, 40)
            btn.setObjectName(f"SourceBtn_{account.account_id}")
            btn.setProperty("account_id", account.account_id)
            
            # Default style
            btn.setStyleSheet(self._get_source_button_style(False))
            
            self.source_button_group.addButton(btn)
            source_layout.addWidget(btn)
            
            # Set first account as default
            if account.account_id == self.selected_account_id:
                btn.setChecked(True)
                btn.setStyleSheet(self._get_source_button_style(True))

        source_layout.addStretch()
        scroll_source.setWidget(source_widget)
        parent_layout.addWidget(scroll_source)

        # --- 2. Kategori Transaksi (QComboBox) ---
        label_tujuan = QLabel("Kategori Transaksi") 
        label_tujuan.setStyleSheet(label_style)
        parent_layout.addWidget(label_tujuan)
        
        self.combo_tujuan = QComboBox()
        self.combo_tujuan.addItems(["Gacha Genshin", "Bayar Listrik", "Transfer ke Ibu", "Gaji", "Bonus", "Lainnya..."]) 
        self.combo_tujuan.setStyleSheet(input_style + dropdown_view_style)
        parent_layout.addWidget(self.combo_tujuan)
        
        # --- 3. Deskripsi Transaksi (QLineEdit) ---
        label_deskripsi = QLabel("Deskripsi Transaksi")
        label_deskripsi.setStyleSheet(label_style)
        parent_layout.addWidget(label_deskripsi)
        
        self.input_deskripsi = QLineEdit()
        self.input_deskripsi.setPlaceholderText("Tulis deskripsi transaksi (e.g., pembeli: Budi, nomor HP: 0812345678)")
        self.input_deskripsi.setStyleSheet(input_style)
        parent_layout.addWidget(self.input_deskripsi)
        
        # --- 4. Nominal (QLineEdit) ---
        label_nominal = QLabel("Nominal")
        label_nominal.setStyleSheet(label_style)
        parent_layout.addWidget(label_nominal)

        self.input_nominal = QLineEdit()
        self.input_nominal.setPlaceholderText("Masukkan nominal (e.g., 200000)")
        self.input_nominal.setValidator(QIntValidator(0, 999999999)) 
        self.input_nominal.setStyleSheet(input_style)
        parent_layout.addWidget(self.input_nominal)

        parent_layout.addSpacerItem(QSpacerItem(20, 20, QSizePolicy.Policy.Minimum, QSizePolicy.Policy.Expanding))
        
    def _get_source_button_style(self, is_checked):
        """Mengembalikan CSS untuk tombol Rekening Sumber berdasarkan status checked."""
        if is_checked:
            return """
                QPushButton { 
                    background-color: #5873E8; 
                    color: white; 
                    border: none; 
                    border-radius: 8px; 
                    font-weight: bold; 
                } 
            """
        else:
            return """
                QPushButton { 
                    background-color: white; 
                    color: #5873E8; 
                    border: 1px solid #5873E8; 
                    border-radius: 8px; 
                    font-weight: 500; 
                } 
                QPushButton:hover { background-color: #F0F2FF; }
            """

    def handleSourceSelection(self, button):
        """Update selected account when source button is clicked."""
        if button.isChecked():
            self.selected_account_id = button.property("account_id")
            self.selected_account_name = button.text()
        
        # Update styles for all buttons in the group
        for btn in self.source_button_group.buttons():
            btn.setStyleSheet(self._get_source_button_style(btn.isChecked()))


    def showEvent(self, event):
        """Called when the view is shown - refresh user accounts from database."""
        super().showEvent(event)
        # Reload user accounts every time the view is shown to ensure latest data
        # This ensures deleted accounts won't appear in the list
        old_accounts = self.user_accounts
        self._load_user_accounts()
        
        # If accounts changed (deleted/added), refresh the buttons
        if len(old_accounts) != len(self.user_accounts):
            # Recreate the form with new accounts - we need to rebuild the source buttons
            # For simplicity, recreate the entire form layout
            self._recreate_account_buttons()

    def _recreate_account_buttons(self):
        """Recreate account selection buttons with current user accounts."""
        # Clear existing buttons from button group
        buttons_to_remove = list(self.source_button_group.buttons())
        for btn in buttons_to_remove:
            self.source_button_group.removeButton(btn)
            btn.deleteLater()
        
        # Find the scroll area containing source buttons and recreate
        # For now, we'll just update the selected account if it's deleted
        if self.user_accounts:
            # Check if currently selected account still exists
            account_ids = [acc.account_id for acc in self.user_accounts]
            if self.selected_account_id not in account_ids:
                # Selected account was deleted, select first account
                self.selected_account_id = self.user_accounts[0].account_id
                self.selected_account_name = self.user_accounts[0].account_name

    def setupConnections(self): 
        """Menghubungkan sinyal dan slot."""
        self.btn_back.clicked.connect(self.back_to_previous.emit)
        self.btn_konfirmasi.clicked.connect(self.onKonfirmasiClicked)
        
        # Hubungkan sinyal dari QButtonGroup (Tipe Transaksi)
        for btn in self.type_button_group.buttons():
            btn.toggled.connect(lambda checked, b=btn: self.handleTypeSelection(b))
            
        # Hubungkan sinyal dari QButtonGroup (Rekening Sumber)
        for btn in self.source_button_group.buttons():
            btn.toggled.connect(lambda checked, b=btn: self.handleSourceSelection(b))

    # --- Step Flow Handlers ---
    def onKonfirmasiClicked(self):
        """Validate input and show confirmation dialog."""
        
        nominal_str = self.input_nominal.text()
        current_nominal = int(nominal_str) if nominal_str.isdigit() else 0

        # Validation
        if not self.transaction_type or not self.selected_account_id or not self.combo_tujuan.currentText() or current_nominal <= 0:
            msg = QMessageBox(self)
            msg.setWindowTitle("Peringatan Input")
            msg.setText("Mohon lengkapi semua field (Tipe Transaksi, Rekening Sumber, Kategori, dan Nominal harus lebih dari 0).")
            msg.setIcon(QMessageBox.Icon.Warning)
            msg.setStyleSheet("""
                QMessageBox { background-color: white; } 
                QLabel { color: black; }
                QPushButton { background-color: #5873E8; color: white; border-radius: 5px; padding: 5px 15px; }
            """)
            msg.exec()
            return
        
        # Prepare data for confirmation dialog
        description = self.input_deskripsi.text().strip() or "Tidak ada deskripsi"
        data = {
            "tipe_transaksi": self.transaction_type, 
            "sumber": self.selected_account_name,
            "tujuan": self.combo_tujuan.currentText(),
            "nominal": current_nominal,
            "deskripsi": description,
        }
        
        # Store data for later processing
        if self.edit_mode and self.edit_transaction_id:
            # Edit mode - use existing transaction ID
            self.transaction_data = {
                "transaction_id": self.edit_transaction_id,
                "account_id": self.selected_account_id,
                "amount": current_nominal,
                "type": self.transaction_type,
                "description": description,
                "category": self.combo_tujuan.currentText(),
            }
        else:
            # Create mode - generate new transaction ID
            self.transaction_data = {
                "transaction_id": str(uuid.uuid4()),
                "account_id": self.selected_account_id,
                "amount": current_nominal,
                "type": self.transaction_type,
                "description": description,
                "category": self.combo_tujuan.currentText(),
            }
        
        conf_dialog = KonfirmasiTransaksiDialog(data, self)
        self._center_dialog(conf_dialog)

        if conf_dialog.exec() == QDialog.DialogCode.Accepted: 
            self.onKonfirmasiLanjut()
    
    def set_edit_mode(self, transaction_id: str, transaction_data: dict):
        """Set the view to edit mode with existing transaction data."""
        # Refresh accounts first to ensure we have latest data
        self._load_user_accounts()
        
        self.edit_mode = True
        self.edit_transaction_id = transaction_id
        self.edit_transaction_data = transaction_data
        
        # Update title
        self.title_label.setText("Edit Transaksi")
        
        # Pre-fill form with transaction data
        # Set transaction type
        trans_type = transaction_data.get("type", "expense")
        if trans_type == "income":
            self.transaction_type = "Pendapatan"
        else:
            self.transaction_type = "Pengeluaran"
        
        # Update type button selection
        for btn in self.type_button_group.buttons():
            if btn.text() == self.transaction_type:
                btn.setChecked(True)
            else:
                btn.setChecked(False)
            btn.setStyleSheet(self._get_type_button_style(btn.isChecked(), btn.text()))
        
        # Set account selection
        account_id = transaction_data.get("account_id")
        if account_id:
            self.selected_account_id = account_id
            # Find account name
            for acc in self.user_accounts:
                if acc.account_id == account_id:
                    self.selected_account_name = acc.account_name
                    break
            
            # Update source button selection
            for btn in self.source_button_group.buttons():
                if btn.property("account_id") == account_id:
                    btn.setChecked(True)
                else:
                    btn.setChecked(False)
                btn.setStyleSheet(self._get_source_button_style(btn.isChecked()))
        
        # Set description
        description = transaction_data.get("description", "")
        self.input_deskripsi.setText(description)
        
        # Set nominal
        amount = transaction_data.get("amount", 0)
        self.input_nominal.setText(str(int(amount)))
        
        # Try to match category in combo box
        # For now, just set to first item or find match in description
        self.combo_tujuan.setCurrentIndex(0)
    
    def clear_edit_mode(self):
        """Clear edit mode and reset to create mode."""
        self.edit_mode = False
        self.edit_transaction_id = None
        self.edit_transaction_data = None
        self.title_label.setText("Tambah Transaksi")
        self.refresh()
    
    def refresh(self):
        """Refresh user accounts and reload form - call this when switching to this view."""
        self._load_user_accounts()
        self.resetForm()

    def onKonfirmasiLanjut(self):
        """Process transaction and add/update it in database."""
        if not hasattr(self, 'transaction_data') or not self.transaction_data:
            msg = QMessageBox(self)
            msg.setWindowTitle("Kesalahan Data")
            msg.setText("Data transaksi tidak valid.")
            msg.setIcon(QMessageBox.Icon.Critical)
            msg.exec()
            return
        
        try:
            # Get the selected account object
            account = None
            for acc in self.user_accounts:
                if acc.account_id == self.selected_account_id:
                    account = acc
                    break
            
            if not account:
                msg = QMessageBox(self)
                msg.setWindowTitle("Kesalahan")
                msg.setText("Akun tidak ditemukan.")
                msg.setIcon(QMessageBox.Icon.Critical)
                msg.exec()
                return
            
            # Map transaction type to enum
            type_mapping = {
                "Pendapatan": TransactionType.INCOME,
                "Pengeluaran": TransactionType.EXPENSE,
            }
            transaction_type = type_mapping.get(self.transaction_data["type"], TransactionType.EXPENSE)
            
            # Check if balance is sufficient for expenses
            if transaction_type == TransactionType.EXPENSE:
                if self.transaction_data["amount"] > account.get_balance():
                    error_dialog = SaldoTidakCukupDialog(
                        saldo_tersedia=account.get_balance(),
                        nominal_transaksi=self.transaction_data["amount"],
                        nama_rekening=account.account_name,
                        parent=self
                    )
                    error_dialog.return_to_form.connect(self.onReturnToForm)
                    self._center_dialog(error_dialog)
                    error_dialog.exec()
                    return
            
            if self.edit_mode and self.edit_transaction_id:
                # Update existing transaction
                success = self.transaction_controller.update_transaction(
                    transaction_id=self.edit_transaction_id,
                    new_amount=self.transaction_data["amount"],
                    new_type=transaction_type,
                    new_description=self.transaction_data["description"],
                    account=account
                )
            else:
                # Add new transaction
                result = self.transaction_controller.add_transaction(
                    transaction_id=self.transaction_data["transaction_id"],
                    account_id=self.transaction_data["account_id"],
                    amount=self.transaction_data["amount"],
                    transaction_type=transaction_type,
                    description=self.transaction_data["description"],
                    account=account
                )
                
                # Handle both old return (bool) and new return (tuple) formats
                if isinstance(result, tuple):
                    success, achieved_targets = result
                else:
                    success = result
                    achieved_targets = []
            
            if success:
                # Show notifications for achieved targets
                if achieved_targets:
                    for target in achieved_targets:
                        self._show_target_achieved_notification(target)
                
                self.showSuccessDialog()
                self.clear_edit_mode()
            else:
                msg = QMessageBox(self)
                msg.setWindowTitle("Kesalahan Transaksi")
                msg.setText("Gagal memproses transaksi. Silakan coba lagi.")
                msg.setIcon(QMessageBox.Icon.Critical)
                msg.exec()
        except Exception as e:
            print(f"Error processing transaction: {e}")
            msg = QMessageBox(self)
            msg.setWindowTitle("Kesalahan")
            msg.setText(f"Kesalahan saat memproses transaksi: {str(e)}")
            msg.setIcon(QMessageBox.Icon.Critical)
            msg.exec()

    def checkPassword(self, password):
        """Check password and show success/error dialog (deprecated - now handled in onKonfirmasiLanjut)."""
        # This method is kept for compatibility but not used anymore
        pass

    def showSuccessDialog(self):
        """Show success dialog and emit signal to go back after closing."""
        success_dialog = SelesaiDialog(self)
        self._center_dialog(success_dialog)
        success_dialog.open()
        
        # Set timer untuk menutup dialog setelah 1.5 detik dan emit signal
        QTimer.singleShot(1500, lambda: self._on_success_dialog_close(success_dialog))
    
    def _on_success_dialog_close(self, dialog):
        """Called when success dialog closes - emit signal to return to main view."""
        dialog.close()
        self.transaction_completed.emit()
    
    def _show_target_achieved_notification(self, target):
        """Show notification dialog when a target is achieved."""
        from views.RekeningView import TargetAchievedDialog
        
        # Get account name for the target
        account_name = "Rekening"
        if self.account_controller:
            try:
                account = self.account_controller.get_account(target.account_id)
                if account:
                    account_name = account.account_name
            except:
                pass
        
        # Show the notification dialog
        notification_dialog = TargetAchievedDialog(self, target.target_name, account_name)
        self._center_dialog(notification_dialog)
        notification_dialog.exec()

    def onReturnToForm(self):
        """Handler saat user memilih kembali dari dialog error."""
        # Tidak perlu ada aksi khusus, karena form utama tidak pernah ditutup
        pass

    def resetForm(self):
        """Reset all form inputs to default values."""
        # Reset QLineEdit
        self.input_nominal.clear()
        self.input_deskripsi.clear()
        
        # Reset QComboBox ke item pertama
        self.combo_tujuan.setCurrentIndex(0)
        
        # Reset Tipe Transaksi (misalnya ke Pengeluaran)
        for btn in self.type_button_group.buttons():
            if btn.text() == "Pengeluaran":
                btn.setChecked(True)
            else:
                btn.setChecked(False)
        
        # Rebuild Rekening Sumber buttons (in case accounts have changed)
        self._rebuild_source_buttons()
    
    def _rebuild_source_buttons(self):
        """Rebuild source account buttons based on current user_accounts."""
        # Remove old buttons from group
        buttons_to_remove = list(self.source_button_group.buttons())
        for btn in buttons_to_remove:
            self.source_button_group.removeButton(btn)
            btn.deleteLater()
        
        # Find the source button container widget
        # We need to find the scroll widget that contains source buttons
        # Let's iterate through the parent layout to find it
        if not hasattr(self, 'source_scroll') or not self.source_scroll:
            return
        
        # Get the widget inside scroll area
        scroll_widget = self.source_scroll.widget()
        if scroll_widget:
            # Clear old layout
            old_layout = scroll_widget.layout()
            if old_layout:
                while old_layout.count():
                    item = old_layout.takeAt(0)
                    if item.widget():
                        item.widget().deleteLater()
        
        # Create new source widget with updated accounts
        source_widget = QWidget()
        source_layout = QHBoxLayout(source_widget)
        source_layout.setContentsMargins(0, 0, 0, 0)
        
        # Recreate buttons based on current user_accounts
        for account in self.user_accounts:
            btn = QPushButton(account.account_name)
            btn.setCheckable(True)
            btn.setFixedSize(120, 40)
            btn.setObjectName(f"SourceBtn_{account.account_id}")
            btn.setProperty("account_id", account.account_id)
            
            # Default style
            btn.setStyleSheet(self._get_source_button_style(False))
            
            self.source_button_group.addButton(btn)
            source_layout.addWidget(btn)
            
            # Set first account as default
            if account.account_id == self.selected_account_id:
                btn.setChecked(True)
                btn.setStyleSheet(self._get_source_button_style(True))
                self.selected_account_name = account.account_name
        
        source_layout.addStretch()
        self.source_scroll.setWidget(source_widget)
        
        # Reconnect signals for newly created buttons
        for btn in self.source_button_group.buttons():
            btn.toggled.connect(lambda checked, b=btn: self.handleSourceSelection(b))

    def _center_dialog(self, dialog):
        """Memposisikan dialog di tengah-tengah jendela utama."""
        parent_rect = self.geometry()
        dialog_width = dialog.width()
        dialog_height = dialog.height()
        
        x = parent_rect.x() + (parent_rect.width() - dialog_width) // 2
        y = parent_rect.y() + (parent_rect.height() - dialog_height) // 2
        
        dialog.move(x, y)


if __name__ == '__main__':
    app = QApplication(sys.argv)
    
    # Untuk menjalankan aplikasi secara mandiri (opsional)
    # Jika Anda menjalankan ini di lingkungan yang sudah menyediakan QApplication, 
    # bagian ini mungkin tidak diperlukan.
    
    view = TransaksiView()
    view.show()
    sys.exit(app.exec())