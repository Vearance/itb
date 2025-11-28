from PyQt6.QtWidgets import (
    QWidget, QLabel, QPushButton, QVBoxLayout, QHBoxLayout,
    QFrame, QScrollArea, QSizePolicy, QMessageBox, QDialog
)
from PyQt6.QtCore import Qt, QSize, pyqtSignal
from PyQt6.QtGui import QIcon


class KonfirmasiHapusDialog(QDialog):
    """Custom dialog untuk konfirmasi hapus rekening"""
    
    def __init__(self, parent, account_name):
        super().__init__(parent)
        self.account_name = account_name
        self.result = False
        self.initUI()
    
    def initUI(self):
        """Setup UI untuk dialog"""
        self.setWindowTitle("Konfirmasi Hapus")
        self.setGeometry(0, 0, 400, 180)
        
        # Center dialog on parent
        if self.parent():
            parent_geo = self.parent().geometry()
            x = parent_geo.x() + (parent_geo.width() - 400) // 2
            y = parent_geo.y() + (parent_geo.height() - 180) // 2
            self.move(x, y)
        
        layout = QVBoxLayout()
        layout.setContentsMargins(20, 20, 20, 20)
        layout.setSpacing(15)
        
        # Message label
        message_label = QLabel(
            f"Apakah Anda yakin ingin menghapus rekening '{self.account_name}'?"
        )
        message_label.setStyleSheet("""
            color: black;
            font-size: 13px;
            background-color: white;
            border: none;
        """)
        message_label.setWordWrap(True)
        layout.addWidget(message_label)
        
        # Buttons layout
        buttons_layout = QHBoxLayout()
        buttons_layout.setSpacing(10)
        buttons_layout.addStretch()
        
        # Cancel button
        cancel_btn = QPushButton("Batal")
        cancel_btn.setFixedWidth(100)
        cancel_btn.setStyleSheet("""
            QPushButton {
                background-color: #f0f0f0;
                color: black;
                border: 1px solid #ddd;
                border-radius: 4px;
                padding: 8px;
                font-size: 12px;
            }
            QPushButton:hover {
                background-color: #e0e0e0;
            }
        """)
        cancel_btn.clicked.connect(self.reject)
        buttons_layout.addWidget(cancel_btn)
        
        # Delete button
        delete_btn = QPushButton("Hapus")
        delete_btn.setFixedWidth(100)
        delete_btn.setStyleSheet("""
            QPushButton {
                background-color: #dc3545;
                color: white;
                border: none;
                border-radius: 4px;
                padding: 8px;
                font-size: 12px;
                font-weight: bold;
            }
            QPushButton:hover {
                background-color: #c82333;
            }
        """)
        delete_btn.clicked.connect(self.accept)
        buttons_layout.addWidget(delete_btn)
        
        layout.addLayout(buttons_layout)
        self.setLayout(layout)
        
        # Set stylesheet untuk dialog
        self.setStyleSheet("""
            QDialog {
                background-color: white;
                border: 1px solid #ddd;
                border-radius: 4px;
            }
        """)


class SelesaiDialog(QDialog):
    """Custom dialog untuk menampilkan pesan sukses"""
    
    def __init__(self, parent, title, message):
        super().__init__(parent)
        self.title_text = title
        self.message_text = message
        self.initUI()
    
    def initUI(self):
        """Setup UI untuk dialog"""
        self.setWindowTitle(self.title_text)
        self.setGeometry(0, 0, 400, 200)
        
        # Center dialog on parent
        if self.parent():
            parent_geo = self.parent().geometry()
            x = parent_geo.x() + (parent_geo.width() - 400) // 2
            y = parent_geo.y() + (parent_geo.height() - 200) // 2
            self.move(x, y)
        
        layout = QVBoxLayout()
        layout.setContentsMargins(20, 30, 20, 30)
        layout.setSpacing(20)
        
        # Success icon label (using text instead of icon)
        icon_label = QLabel("✓")
        icon_label.setStyleSheet("""
            color: #28a745;
            font-size: 48px;
            font-weight: bold;
            background-color: white;
            border: none;
        """)
        icon_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        layout.addWidget(icon_label)
        
        # Message label
        message_label = QLabel(self.message_text)
        message_label.setStyleSheet("""
            color: black;
            font-size: 13px;
            background-color: white;
            border: none;
        """)
        message_label.setWordWrap(True)
        message_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        layout.addWidget(message_label)
        
        # OK button
        ok_btn = QPushButton("OK")
        ok_btn.setFixedWidth(100)
        ok_btn.setStyleSheet("""
            QPushButton {
                background-color: #28a745;
                color: white;
                border: none;
                border-radius: 4px;
                padding: 8px;
                font-size: 12px;
                font-weight: bold;
            }
            QPushButton:hover {
                background-color: #218838;
            }
        """)
        ok_btn.clicked.connect(self.accept)
        
        button_layout = QHBoxLayout()
        button_layout.addStretch()
        button_layout.addWidget(ok_btn)
        button_layout.addStretch()
        layout.addLayout(button_layout)
        
        self.setLayout(layout)
        
        # Set stylesheet untuk dialog
        self.setStyleSheet("""
            QDialog {
                background-color: white;
                border: 1px solid #ddd;
                border-radius: 4px;
            }
        """)


class WarningDialog(QDialog):
    """Custom dialog untuk menampilkan pesan warning"""
    
    def __init__(self, parent, title, message):
        super().__init__(parent)
        self.title_text = title
        self.message_text = message
        self.initUI()
    
    def initUI(self):
        """Setup UI untuk dialog"""
        self.setWindowTitle(self.title_text)
        self.setGeometry(0, 0, 400, 220)
        
        # Center dialog on parent
        if self.parent():
            parent_geo = self.parent().geometry()
            x = parent_geo.x() + (parent_geo.width() - 400) // 2
            y = parent_geo.y() + (parent_geo.height() - 220) // 2
            self.move(x, y)
        
        layout = QVBoxLayout()
        layout.setContentsMargins(20, 30, 20, 30)
        layout.setSpacing(20)
        
        # Warning icon label (using text instead of icon)
        icon_label = QLabel("⚠")
        icon_label.setStyleSheet("""
            color: #dc3545;
            font-size: 48px;
            font-weight: bold;
            background-color: white;
            border: none;
        """)
        icon_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        layout.addWidget(icon_label)
        
        # Message label
        message_label = QLabel(self.message_text)
        message_label.setStyleSheet("""
            color: black;
            font-size: 13px;
            background-color: white;
            border: none;
        """)
        message_label.setWordWrap(True)
        message_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        layout.addWidget(message_label)
        
        # OK button
        ok_btn = QPushButton("OK")
        ok_btn.setFixedWidth(100)
        ok_btn.setStyleSheet("""
            QPushButton {
                background-color: #dc3545;
                color: white;
                border: none;
                border-radius: 4px;
                padding: 8px;
                font-size: 12px;
                font-weight: bold;
            }
            QPushButton:hover {
                background-color: #c82333;
            }
        """)
        ok_btn.clicked.connect(self.accept)
        
        button_layout = QHBoxLayout()
        button_layout.addStretch()
        button_layout.addWidget(ok_btn)
        button_layout.addStretch()
        layout.addLayout(button_layout)
        
        self.setLayout(layout)
        
        # Set stylesheet untuk dialog
        self.setStyleSheet("""
            QDialog {
                background-color: white;
                border: 1px solid #ddd;
                border-radius: 4px;
            }
        """)


class TargetAchievedDialog(QDialog):
    """Custom dialog untuk menampilkan notifikasi target terpenuhi"""
    
    def __init__(self, parent, target_name, account_name):
        super().__init__(parent)
        self.target_name = target_name
        self.account_name = account_name
        self.initUI()
    
    def initUI(self):
        """Setup UI untuk dialog"""
        self.setWindowTitle("Target Terpenuhi!")
        self.setGeometry(0, 0, 450, 250)
        
        # Center dialog on parent
        if self.parent():
            parent_geo = self.parent().geometry()
            x = parent_geo.x() + (parent_geo.width() - 450) // 2
            y = parent_geo.y() + (parent_geo.height() - 250) // 2
            self.move(x, y)
        
        layout = QVBoxLayout()
        layout.setContentsMargins(30, 30, 30, 30)
        layout.setSpacing(20)
        
        # Success icon label
        icon_label = QLabel("🎉")
        icon_label.setStyleSheet("""
            color: #28a745;
            font-size: 64px;
            background-color: white;
            border: none;
        """)
        icon_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        layout.addWidget(icon_label)
        
        # Title label
        title_label = QLabel("Selamat!")
        title_label.setStyleSheet("""
            color: #28a745;
            font-size: 20px;
            font-weight: bold;
            background-color: white;
            border: none;
        """)
        title_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        layout.addWidget(title_label)
        
        # Message label
        message_text = f"Target <b>'{self.target_name}'</b> di rekening <b>'{self.account_name}'</b> telah terpenuhi!"
        message_label = QLabel(message_text)
        message_label.setStyleSheet("""
            color: black;
            font-size: 14px;
            background-color: white;
            border: none;
        """)
        message_label.setWordWrap(True)
        message_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        layout.addWidget(message_label)
        
        # OK button
        ok_btn = QPushButton("OK")
        ok_btn.setFixedWidth(120)
        ok_btn.setStyleSheet("""
            QPushButton {
                background-color: #28a745;
                color: white;
                border: none;
                border-radius: 6px;
                padding: 10px;
                font-size: 13px;
                font-weight: bold;
            }
            QPushButton:hover {
                background-color: #218838;
            }
        """)
        ok_btn.clicked.connect(self.accept)
        
        button_layout = QHBoxLayout()
        button_layout.addStretch()
        button_layout.addWidget(ok_btn)
        button_layout.addStretch()
        layout.addLayout(button_layout)
        
        self.setLayout(layout)
        
        # Set stylesheet untuk dialog
        self.setStyleSheet("""
            QDialog {
                background-color: white;
                border: 1px solid #28a745;
                border-radius: 8px;
            }
        """)


class RekeningView(QWidget):
    """View untuk menampilkan saldo rekening dan detail transaksi per rekening"""
    
    add_account_requested = pyqtSignal()
    transaction_requested = pyqtSignal(str)  # Emit account_id when transaction button clicked
    account_edit_requested = pyqtSignal(str, dict)  # Emit (account_id, account_data) for edit
    account_deleted = pyqtSignal()  # Emit when account is successfully deleted
    
    def __init__(self, user, account_controller, transaction_controller, target_controller=None):
        super().__init__()
        self.user = user
        self.account_controller = account_controller
        self.transaction_controller = transaction_controller
        self.target_controller = target_controller
        self.initUI()
    
    def initUI(self):
        """Inisialisasi UI"""
        self.setStyleSheet("background-color: white;")
        
        main_layout = QHBoxLayout()
        main_layout.setContentsMargins(0, 0, 0, 0)
        main_layout.setSpacing(0)
        self.setLayout(main_layout)
        
        # Scroll Area untuk konten
        scroll_area = QScrollArea()
        scroll_area.setWidgetResizable(True)
        scroll_area.setStyleSheet("""
            QScrollArea { border: none; background-color: white; }
            QScrollBar:vertical { width: 10px; }
        """)
        
        content_widget = QWidget()
        self.content_layout = QVBoxLayout(content_widget)
        self.content_layout.setContentsMargins(40, 40, 40, 40)
        self.content_layout.setSpacing(25)
        
        # Header
        header = self.createHeader()
        self.content_layout.addWidget(header)
        
        # Saldo Cards Section
        saldo_section = self.createSaldoSection()
        self.content_layout.addWidget(saldo_section)
        
        # Detail Pengeluaran dan Transaksi per Rekening
        detail_section = self.createDetailSection()
        self.content_layout.addWidget(detail_section, 1)
        
        scroll_area.setWidget(content_widget)
        main_layout.addWidget(scroll_area)

    def refresh(self):
        """Refresh tampilan dengan data terbaru"""
        # Hapus widget lama
        while self.content_layout.count():
            child = self.content_layout.takeAt(0)
            if child.widget():
                child.widget().deleteLater()
        
        # Rebuild UI
        self.content_layout.setContentsMargins(40, 40, 40, 40)
        self.content_layout.setSpacing(25)
        
        # Header
        header = self.createHeader()
        self.content_layout.addWidget(header)
        
        # Saldo Cards Section
        saldo_section = self.createSaldoSection()
        self.content_layout.addWidget(saldo_section)
        
        # Detail Pengeluaran dan Transaksi per Rekening
        detail_section = self.createDetailSection()
        self.content_layout.addWidget(detail_section, 1)
    
    def createHeader(self):
        """Header section sudah di dalam saldo section, jadi ini kosong atau minimal"""
        header_frame = QFrame()
        header_frame.setStyleSheet("border: none;")
        header_frame.setFixedHeight(0)  # No separate header needed
        return header_frame
    
    def createSaldoSection(self):
        """Buat section saldo cards horizontal dengan header di dalamnya"""
        section = QFrame()
        section.setStyleSheet("""
            QFrame {
                background-color: #5873E8;
                border-radius: 15px;
            }
        """)
        
        main_layout = QVBoxLayout(section)
        main_layout.setContentsMargins(30, 25, 30, 25)
        main_layout.setSpacing(20)
        
        # Header di dalam section biru
        header_label = QLabel("Saldo di e-wallet kamu")
        header_label.setStyleSheet("color: white; font-size: 20px; font-weight: bold; border: none;")
        main_layout.addWidget(header_label)
        
        # Scroll Area untuk cards yang banyak
        scroll_area = QScrollArea()
        scroll_area.setWidgetResizable(True)
        scroll_area.setHorizontalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAsNeeded)
        scroll_area.setVerticalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAlwaysOff)
        scroll_area.setFixedHeight(120)
        scroll_area.setStyleSheet("""
            QScrollArea { 
                border: none; 
                background-color: #5873E8;
            }
            QScrollBar:horizontal {
                border: none;
                background: rgba(255, 255, 255, 0.1);
                height: 8px;
                margin: 0px 0px 0px 0px;
                border-radius: 4px;
            }
            QScrollBar::handle:horizontal {
                background: rgba(255, 255, 255, 0.3);
                min-width: 20px;
                border-radius: 4px;
            }
            QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
                border: none;
                background: none;
            }
        """)
        
        # Container widget untuk scroll area
        scroll_content = QWidget()
        scroll_content.setStyleSheet("background-color: #5873E8;")
        cards_layout = QHBoxLayout(scroll_content)
        cards_layout.setContentsMargins(0, 0, 0, 0)
        cards_layout.setSpacing(15)
        
        # Get user accounts
        accounts = []
        if self.account_controller:
            accounts = self.account_controller.get_user_accounts(self.user.user_id)
        
        if not accounts or len(accounts) == 0:
            # No accounts - show single empty card
            empty_card = QFrame()
            empty_card.setFixedSize(160, 70)
            empty_card.setStyleSheet("""
                QFrame {
                    background-color: white;
                    border-radius: 12px;
                }
            """)
            empty_layout = QVBoxLayout(empty_card)
            empty_layout.setAlignment(Qt.AlignmentFlag.AlignCenter)
            
            empty_label = QLabel("Belum ada rekening")
            empty_label.setStyleSheet("color: #999; font-size: 12px; border: none;")
            empty_layout.addWidget(empty_label)
            
            cards_layout.addWidget(empty_card)
        else:
            # Display account cards
            for account in accounts:
                card = self.createSaldoCard(account)
                cards_layout.addWidget(card)
        
        # Add button (always show)
        add_btn = QPushButton()
        add_btn.setFixedSize(70, 70)
        
        # Use TambahRekening.svg
        icon = QIcon("img/TambahRekening.svg")
        add_btn.setIcon(icon)
        add_btn.setIconSize(QSize(70, 70))
        
        add_btn.setStyleSheet("""
            QPushButton {
                background-color: transparent;
                border: none;
                border-radius: 12px;
            }
            QPushButton:hover {
                background-color: rgba(255, 255, 255, 0.1);
            }
        """)
        add_btn.setCursor(Qt.CursorShape.PointingHandCursor)
        add_btn.clicked.connect(self.add_account_requested.emit)
        cards_layout.addWidget(add_btn)
        
        cards_layout.addStretch()
        scroll_area.setWidget(scroll_content)
        main_layout.addWidget(scroll_area)
        
        return section
    
    def createSaldoCard(self, account):
        """Buat card saldo individual dengan icon wallet"""
        card = QFrame()
        card.setFixedSize(160, 70)
        card.setStyleSheet("""
            QFrame {
                background-color: white;
                border-radius: 12px;
            }
        """)
        
        layout = QHBoxLayout(card)
        layout.setContentsMargins(12, 10, 12, 10)
        layout.setSpacing(8)
        
        # Wallet icon (using Rekening.svg)
        icon_label = QLabel()
        icon = QIcon("img/Rekening.svg")
        if not icon.isNull():
            pixmap = icon.pixmap(QSize(24, 24))
            icon_label.setPixmap(pixmap)
        else:
            icon_label.setText("📁")  # Fallback
        icon_label.setFixedSize(24, 24)
        layout.addWidget(icon_label)
        
        # Text layout
        text_layout = QVBoxLayout()
        text_layout.setSpacing(2)
        
        # Full account name with e-wallet type
        # Format: "Nama Rekening (JENIS)"
        display_name = account.account_name
        
        name_label = QLabel(display_name)
        name_label.setStyleSheet("color: #5873E8; font-size: 13px; font-weight: bold; border: none;")
        name_label.setWordWrap(True)
        
        # Balance
        balance_label = QLabel(f"Rp{account.balance:,.0f}")
        balance_label.setStyleSheet("color: #5873E8; font-size: 12px; border: none;")
        
        text_layout.addWidget(name_label)
        text_layout.addWidget(balance_label)
        
        layout.addLayout(text_layout)
        layout.addStretch()
        
        return card
    
    def createDetailSection(self):
        """Buat section detail per rekening"""
        section = QWidget()
        layout = QVBoxLayout(section)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(20)
        
        # Title
        title = QLabel("Detail Pengeluaran dan Transaksi")
        title.setStyleSheet("font-size: 18px; font-weight: bold; color: black;")
        layout.addWidget(title)
        
        # Get user accounts
        accounts = []
        if self.account_controller:
            accounts = self.account_controller.get_user_accounts(self.user.user_id)
        
        if not accounts or len(accounts) == 0:
            empty_label = QLabel("Tidak ada data rekening")
            empty_label.setStyleSheet("color: grey; font-style: italic;")
            layout.addWidget(empty_label)
        else:
            # Create detail card for each account
            for account in accounts:
                detail_card = self.createAccountDetailCard(account)
                layout.addWidget(detail_card)
        
        layout.addStretch()
        return section
    
    def createAccountDetailCard(self, account):
        """Buat card detail untuk satu rekening"""
        card = QFrame()
        card.setStyleSheet("""
            QFrame {
                background-color: #F9F9F9;
                border-radius: 12px;
                padding: 15px;
            }
            QFrame:hover {
                background-color: #F0F2FF;
            }
        """)
        card.setCursor(Qt.CursorShape.PointingHandCursor)
        card.setToolTip("Double-click untuk edit rekening")
        
        # Add double-click handler for editing
        def mouseDoubleClickEvent(a0):
            self.onAccountDoubleClicked(account)
        card.mouseDoubleClickEvent = mouseDoubleClickEvent
        
        layout = QVBoxLayout(card)
        layout.setContentsMargins(20, 15, 20, 15)
        layout.setSpacing(12)
        
        # Header: Account name + Delete button
        header_layout = QHBoxLayout()
        
        account_label = QLabel(account.account_name)
        account_label.setStyleSheet("font-size: 16px; font-weight: bold; color: black; border: none;")
        
        delete_btn = QPushButton("Hapus rekening")
        delete_btn.setFixedHeight(25)
        delete_btn.setStyleSheet("""
            QPushButton {
                background-color: transparent;
                color: #666;
                border: none;
                font-size: 11px;
                text-align: right;
            }
            QPushButton:hover {
                color: #E74C3C;
                text-decoration: underline;
            }
        """)
        delete_btn.setCursor(Qt.CursorShape.PointingHandCursor)
        delete_btn.clicked.connect(lambda: self.handle_delete_account(account))
        
        header_layout.addWidget(account_label)
        header_layout.addStretch()
        header_layout.addWidget(delete_btn)
        
        layout.addLayout(header_layout)
        
        # Stats cards
        stats_layout = QHBoxLayout()
        stats_layout.setSpacing(10)
        
        # Get transactions for this account
        income_total = 0
        expense_total = 0
        target_progress = "Rp0 / Rp0"
        
        if self.transaction_controller:
            transactions = self.transaction_controller.get_account_transactions(account.account_id)
            for trans in transactions:
                if trans.transaction_type.value == "income":
                    income_total += trans.amount
                elif trans.transaction_type.value == "expense":
                    expense_total += trans.amount
        
        # Get target progress for this account
        if self.target_controller:
            account_targets = self.target_controller.get_account_targets(account.account_id)
            total_current = sum(t.current_amount for t in account_targets if not t.is_archived)
            total_target = sum(t.target_amount for t in account_targets if not t.is_archived)
            target_progress = f"Rp{total_current:,.0f} / Rp{total_target:,.0f}".replace(",", ".")
        
        # Pemasukan card
        pemasukan_card = self.createStatCard("Pemasukan", f"+Rp{income_total:,.0f}", "#2ECC71")
        stats_layout.addWidget(pemasukan_card)
        
        # Pengeluaran card
        pengeluaran_card = self.createStatCard("Pengeluaran", f"-Rp{expense_total:,.0f}", "#E74C3C")
        stats_layout.addWidget(pengeluaran_card)
        
        # Progress tabungan card
        progress_card = self.createStatCard("Progress tabungan saat ini", target_progress, "#5873E8")
        stats_layout.addWidget(progress_card)
        
        layout.addLayout(stats_layout)
        
        # Action button
        action_btn = QPushButton("Lakukan transaksi  >")
        action_btn.setFixedHeight(45)
        action_btn.setStyleSheet("""
            QPushButton {
                background-color: #5873E8;
                color: white;
                border: none;
                border-radius: 8px;
                font-size: 14px;
                font-weight: bold;
            }
            QPushButton:hover {
                background-color: #4a63d0;
            }
        """)
        action_btn.setCursor(Qt.CursorShape.PointingHandCursor)
        action_btn.clicked.connect(lambda: self.transaction_requested.emit(account.account_id))
        
        layout.addWidget(action_btn)
        
        return card
    
    def createStatCard(self, title, value, color):
        """Helper untuk membuat kartu statistik kecil"""
        card = QFrame()
        card.setObjectName("StatCard")
        # Use ID selector to ensure style only applies to the container, not children
        card.setStyleSheet(f"""
            #StatCard {{
                background-color: white;
                border: 1px solid {color};
                border-radius: 8px;
            }}
        """)
        card.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Minimum)
        card.setMinimumWidth(100)
        # Remove fixed/minimum height to let it be as compact as possible
        # card.setMinimumHeight(60) 
        
        layout = QVBoxLayout(card)
        layout.setContentsMargins(10, 8, 10, 8)
        layout.setSpacing(2)
        
        # Title label
        title_label = QLabel(title)
        title_label.setWordWrap(True)
        title_label.setStyleSheet("""
            QLabel {
                color: #333333;
                font-size: 10px;
                font-weight: bold;
                border: none;
                background-color: transparent;
            }
        """)
        title_label.setAlignment(Qt.AlignmentFlag.AlignLeft | Qt.AlignmentFlag.AlignTop)
        layout.addWidget(title_label)
        
        # Check if this is progress card (has "/" in value)
        if "/" in value and title.startswith("Progress"):
            # Add a small spacer
            layout.addSpacing(4)
            
            # Progress bar container
            progress_frame = QFrame()
            progress_frame.setFixedHeight(4)
            progress_frame.setStyleSheet("""
                background-color: #F0F0F0;
                border-radius: 2px;
                border: none;
            """)
            
            # Use layout for progress bar to handle width dynamically
            progress_layout = QHBoxLayout(progress_frame)
            progress_layout.setContentsMargins(0, 0, 0, 0)
            progress_layout.setSpacing(0)
            
            # Inner progress
            progress_inner = QFrame()
            progress_inner.setStyleSheet(f"""
                background-color: {color};
                border-radius: 2px;
                border: none;
            """)
            
            # Calculate ratio (dummy logic for now, e.g. 50%)
            ratio = 0.5 
            
            progress_layout.addWidget(progress_inner, int(ratio * 100))
            progress_layout.addStretch(int((1 - ratio) * 100))
            
            layout.addWidget(progress_frame)
            
            # Add spacing
            layout.addSpacing(4)
            
            # Progress value
            value_label = QLabel(value)
            value_label.setStyleSheet(f"""
                QLabel {{
                    color: {color};
                    font-size: 10px;
                    font-weight: bold;
                    border: none;
                    background-color: transparent;
                }}
            """)
            value_label.setAlignment(Qt.AlignmentFlag.AlignLeft | Qt.AlignmentFlag.AlignBottom)
            layout.addWidget(value_label)
        else:
            # Regular stat card
            # Use stretch to push value to bottom, but since we want compact, 
            # maybe just a small spacing is better if we want it to hug content?
            # The user wants "compact", so let's use a small fixed spacing instead of stretch
            # to avoid it expanding unnecessarily if the container grows.
            # BUT, if the container is forced to be same height as others, stretch is good.
            # Let's use addStretch(1) to be safe but keep min height low.
            layout.addStretch(1)
            
            value_label = QLabel(value)
            value_label.setStyleSheet(f"""
                QLabel {{
                    color: {color};
                    font-size: 13px;
                    font-weight: bold;
                    border: none;
                    background-color: transparent;
                }}
            """)
            value_label.setAlignment(Qt.AlignmentFlag.AlignLeft | Qt.AlignmentFlag.AlignBottom)
            layout.addWidget(value_label)
        
        return card

    def handle_delete_account(self, account):
        """Handle account deletion with confirmation"""
        # Show custom confirmation dialog
        dialog = KonfirmasiHapusDialog(self, account.account_name)
        
        if dialog.exec() != QDialog.DialogCode.Accepted:
            return
        
        # Try to delete the account
        if self.account_controller:
            success = self.account_controller.delete_account(account.account_id, self.user.user_id)
            
            if success:
                dialog = SelesaiDialog(
                    self,
                    "Berhasil",
                    f"Rekening '{account.account_name}' berhasil dihapus."
                )
                dialog.exec()
                # Refresh the view
                self.refresh()
                # Emit signal to notify MainWindow
                self.account_deleted.emit()
            else:
                dialog = WarningDialog(
                    self,
                    "Tidak Dapat Dihapus",
                    "Anda tidak dapat menghapus rekening ini karena Anda harus memiliki minimal 1 rekening aktif."
                )
                dialog.exec()

    def onAccountDoubleClicked(self, account):
        """Handle double-click on account card to edit"""
        # Parse account name to extract ewallet and deskripsi
        # Format: "Deskripsi (EWALLET)"
        account_name = account.account_name
        ewallet = ""
        deskripsi = account_name
        
        # Try to extract ewallet type from account name
        if "(" in account_name and account_name.endswith(")"):
            parts = account_name.rsplit("(", 1)
            deskripsi = parts[0].strip()
            ewallet = parts[1].rstrip(")")
        
        # Prepare account data dict
        account_data = {
            'account_id': account.account_id,
            'ewallet': ewallet,
            'norek': account.account_id,  # Account ID is used as norek
            'nominal': account.balance,
            'deskripsi': deskripsi
        }
        
        # Emit signal with account id and data
        self.account_edit_requested.emit(account.account_id, account_data)
