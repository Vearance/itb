from PyQt6.QtWidgets import (
    QWidget, QLabel, QPushButton, QVBoxLayout, QHBoxLayout,
    QFrame, QLineEdit, QComboBox, QDialog, QSizePolicy, QSpacerItem,
    QMessageBox, QDateEdit, QScrollArea, QCalendarWidget
)
from PyQt6.QtCore import Qt, pyqtSignal, QSize, QTimer, QDate
from PyQt6.QtGui import QIcon, QIntValidator
from datetime import datetime
import uuid

# --- Custom Calendar Widget dengan Styling ---
class StyledCalendarWidget(QCalendarWidget):
    """Custom calendar widget dengan styling yang tepat"""
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setStyleSheet("""
            QCalendarWidget {
                background-color: white;
                color: black;
                border: 1px solid #ddd;
            }
            QCalendarWidget QWidget {
                background-color: white;
                color: black;
            }
            QCalendarWidget QAbstractButton {
                color: black;
                background-color: white;
                border: none;
                padding: 5px;
            }
            QCalendarWidget QAbstractButton:hover {
                background-color: #f0f0f0;
            }
            QCalendarWidget QAbstractButton:pressed {
                background-color: #5873E8;
                color: white;
            }
            QCalendarWidget QMenu {
                background-color: white;
                color: black;
            }
            QCalendarWidget QSpinBox {
                background-color: white;
                color: black;
                border: 1px solid #ddd;
            }
            QCalendarWidget QToolButton {
                color: black;
                background-color: white;
                border: none;
                padding: 5px;
            }
            QCalendarWidget QToolButton:hover {
                background-color: #f0f0f0;
            }
            QCalendarWidget QToolButton:pressed {
                background-color: #5873E8;
            }
        """)

class StyledDateEdit(QDateEdit):
    """Custom date edit dengan styled calendar popup"""
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setCalendarPopup(True)
        # Set custom calendar widget
        calendar = StyledCalendarWidget()
        self.setCalendarWidget(calendar)

# --- A. Pop-up Dialogs untuk Tambah Target ---

class KonfirmasiTargetDialog(QDialog):
    def __init__(self, target_name, ewallet_name, nominal_target, deadline, parent=None):
        super().__init__(parent)
        self.setWindowTitle("Konfirmasi Target")
        self.setFixedSize(400, 340) 
        self.setWindowFlags(self.windowFlags() & ~Qt.WindowType.WindowContextHelpButtonHint) 
        self.initUI(target_name, ewallet_name, nominal_target, deadline)
    
    def create_data_row(self, label_text, value_text):
        h_layout = QHBoxLayout()
        label = QLabel(label_text)
        value = QLabel(value_text)
        value.setObjectName("DataValue")
        value.setAlignment(Qt.AlignmentFlag.AlignRight)
        h_layout.addWidget(label)
        h_layout.addWidget(value)
        return h_layout

    def initUI(self, target_name, ewallet_name, nominal_target, deadline):
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
        
        title = QLabel("Konfirmasi Target Menabung")
        title.setObjectName("HeaderTitle")
        title.setAlignment(Qt.AlignmentFlag.AlignCenter)
        main_layout.addWidget(title)
        
        data_frame = QFrame()
        data_layout = QVBoxLayout(data_frame)
        data_layout.setContentsMargins(0, 10, 0, 10)
        data_layout.setSpacing(10)

        data_layout.addLayout(self.create_data_row("Nama Target", target_name))
        data_layout.addLayout(self.create_data_row("Rekening", ewallet_name))
        data_layout.addLayout(self.create_data_row("Nominal Target", f"Rp{int(nominal_target):,}".replace(",", ".")))
        data_layout.addLayout(self.create_data_row("Deadline", deadline.strftime("%d %B %Y"))) 

        main_layout.addWidget(data_frame)
        main_layout.addStretch()

        button_layout = QHBoxLayout()
        button_layout.setSpacing(10)
        
        btn_kembali = QPushButton("Kembali")
        btn_kembali.setStyleSheet("QPushButton { background-color: white; color: #5873E8; border: 1px solid #C0C0C0; } QPushButton:hover { background-color: #F0F2FF; }")
        btn_kembali.clicked.connect(self.reject) 

        btn_lanjutkan = QPushButton("Buat Target")
        btn_lanjutkan.setStyleSheet("QPushButton { background-color: #5873E8; color: white; } QPushButton:hover { background-color: #4A63D0; }")
        btn_lanjutkan.clicked.connect(self.accept) 
        
        button_layout.addWidget(btn_kembali)
        button_layout.addWidget(btn_lanjutkan)
        
        main_layout.addLayout(button_layout)


class TargetSelesaiDialog(QDialog):
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
                background-color: transparent; /* Penting untuk latar belakang ikon/teks */
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
        
        icon_label = QLabel()
        icon = QIcon("img/Checkmark.svg") 
        icon_label.setPixmap(icon.pixmap(QSize(64, 64))) 
        icon_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        main_layout.addWidget(icon_label)
        
        # Teks sukses yang diperbarui
        message = QLabel("Target berhasil ditambahkan!")
        message.setObjectName("SuccessText")
        message.setAlignment(Qt.AlignmentFlag.AlignCenter)
        main_layout.addWidget(message)
        
        main_layout.addStretch() 

# --- B. Main View: Tambah Target ---
class TambahTargetView(QWidget):
    
    # Signal emitted when target is successfully added
    target_added = pyqtSignal()
    # Signal untuk kembali ke halaman sebelumnya
    back_to_previous = pyqtSignal()

    def __init__(self, user=None, account_controller=None, target_controller=None):
        super().__init__()
        self.user = user
        self.account_controller = account_controller
        self.target_controller = target_controller
        
        # Cache for user accounts
        self.user_accounts = []
        self.selected_account_id = None
        
        # Edit mode properties
        self.edit_mode = False
        self.edit_target_id = None
        self.edit_target_data = None
        
        # Load user accounts
        self._load_user_accounts()
        
        self.initUI()
        self.setupConnections()
    
    def _load_user_accounts(self):
        """Load user accounts from database using account_controller."""
        if self.user and self.account_controller:
            try:
                self.user_accounts = self.account_controller.get_user_accounts(self.user.user_id)
                if self.user_accounts:
                    self.selected_account_id = self.user_accounts[0].account_id
            except Exception as e:
                print(f"Error loading user accounts: {e}")
                self.user_accounts = []
        
    def initUI(self):
        self.setWindowTitle("Tabungin - Tambah Target")
        self.setStyleSheet("background-color: white;")
        self.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Expanding)

        main_layout = QHBoxLayout()
        main_layout.setContentsMargins(0, 0, 0, 0)
        self.setLayout(main_layout)
        
        content_widget = QWidget()
        content_layout = QVBoxLayout(content_widget)
        content_layout.setContentsMargins(50, 40, 50, 40)
        content_layout.setSpacing(25)
        
        # Header dengan tombol kembali dan judul
        header_layout = QHBoxLayout()
        
        # Tombol Kembali (Arrow Left)
        self.btn_back = QPushButton()
        self.btn_back.setFixedSize(32, 32)
        self.btn_back.setCursor(Qt.CursorShape.PointingHandCursor)
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

        self.title_label = QLabel("Tambah Target Menabung")
        self.title_label.setStyleSheet("font-size: 20px; font-weight: bold; color: black;")
        
        header_layout.addWidget(self.title_label)
        header_layout.addStretch()
        
        content_layout.addLayout(header_layout)
        content_layout.addSpacerItem(QSpacerItem(20, 10, QSizePolicy.Policy.Minimum, QSizePolicy.Policy.Fixed))
        
        # Scrollable form area
        scroll_area = QScrollArea()
        scroll_area.setWidgetResizable(True)
        scroll_area.setFrameShape(QFrame.Shape.NoFrame)
        scroll_area.setStyleSheet("background-color: white; border: none;")
        
        form_widget = QWidget()
        self.form_layout = QVBoxLayout(form_widget)
        self.form_layout.setContentsMargins(0, 0, 0, 0)
        self.form_layout.setSpacing(20)
        
        self.createForm(self.form_layout)
        
        scroll_area.setWidget(form_widget)
        content_layout.addWidget(scroll_area)
        
        # Tombol Konfirmasi (outside scroll area)
        self.btn_konfirmasi = QPushButton("Konfirmasi")
        self.btn_konfirmasi.setFixedHeight(50)
        self.btn_konfirmasi.setCursor(Qt.CursorShape.PointingHandCursor)
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

    def createForm(self, parent_layout: QVBoxLayout):
        label_style = "font-size: 14px; font-weight: bold; color: black; margin-bottom: 5px;"
        input_style = """
            QLineEdit, QComboBox, QDateEdit { 
                padding: 8px 10px; 
                border: 1px solid #C0C0C0; 
                border-radius: 8px; 
                font-size: 14px; 
                color: black; 
                background-color: white;
            }
        """
        dropdown_view_style = """
            QComboBox QAbstractItemView { 
                border: 1px solid #C0C0C0; 
                selection-background-color: #5873E8; 
                color: black; 
            }
        """
        
        # 1. Nama Target
        label_target_name = QLabel("Nama Target")
        label_target_name.setStyleSheet(label_style)
        parent_layout.addWidget(label_target_name)
        
        self.input_target_name = QLineEdit()
        self.input_target_name.setPlaceholderText("Masukkan nama target (e.g., Beli iPhone, Liburan)") 
        self.input_target_name.setFixedHeight(45)
        self.input_target_name.setStyleSheet(input_style)
        parent_layout.addWidget(self.input_target_name)
        
        # 2. Pilih Rekening untuk menabung
        label_rekening = QLabel("Pilih Rekening untuk menabung")
        label_rekening.setStyleSheet(label_style)
        parent_layout.addWidget(label_rekening)
        
        self.combo_rekening = QComboBox()
        self.combo_rekening.setFixedHeight(45)
        self.combo_rekening.setEditable(False)
        self.combo_rekening.setStyleSheet(input_style + dropdown_view_style)
        
        # Populate with real user accounts
        if self.user_accounts:
            for account in self.user_accounts:
                self.combo_rekening.addItem(account.account_name, account.account_id)
        else:
            self.combo_rekening.addItem("Tidak ada rekening tersedia", None)
        
        parent_layout.addWidget(self.combo_rekening)
        
        # 3. Nominal Target
        label_nominal = QLabel("Nominal Target")
        label_nominal.setStyleSheet(label_style)
        parent_layout.addWidget(label_nominal)

        self.input_nominal = QLineEdit()
        self.input_nominal.setPlaceholderText("Masukkan nominal target (e.g., 5000000)")
        self.input_nominal.setFixedHeight(45)
        self.input_nominal.setValidator(QIntValidator(0, 999999999)) 
        self.input_nominal.setStyleSheet(input_style)
        parent_layout.addWidget(self.input_nominal)
        
        # 4. Deadline (Date Picker) - NEW!
        label_deadline = QLabel("Deadline Target")
        label_deadline.setStyleSheet(label_style)
        parent_layout.addWidget(label_deadline)
        
        self.date_deadline = StyledDateEdit()
        self.date_deadline.setFixedHeight(45)
        self.date_deadline.setDisplayFormat("dd MMMM yyyy")
        # Set minimum date to tomorrow
        tomorrow = QDate.currentDate().addDays(1)
        self.date_deadline.setMinimumDate(tomorrow)
        # Default to 30 days from now
        self.date_deadline.setDate(QDate.currentDate().addDays(30))
        self.date_deadline.setStyleSheet(input_style + """
            QDateEdit::drop-down {
                border: none;
                width: 30px;
            }
            QDateEdit::down-arrow {
                image: url(img/ChevronDown.svg);
                width: 12px;
                height: 12px;
            }
        """)
        parent_layout.addWidget(self.date_deadline)
        
        parent_layout.addSpacerItem(QSpacerItem(20, 20, QSizePolicy.Policy.Minimum, QSizePolicy.Policy.Expanding))

    def setupConnections(self): 
        self.btn_konfirmasi.clicked.connect(self.onKonfirmasiClicked)
        self.btn_back.clicked.connect(self.back_to_previous.emit)
    
    def showEvent(self, a0):
        """Called when the view is shown - refresh user accounts from database."""
        super().showEvent(a0)
        self._refresh_accounts()
    
    def _refresh_accounts(self):
        """Refresh account dropdown with latest data from database."""
        old_selected = self.combo_rekening.currentData()
        
        # Reload accounts
        self._load_user_accounts()
        
        # Update dropdown
        self.combo_rekening.clear()
        if self.user_accounts:
            for account in self.user_accounts:
                self.combo_rekening.addItem(account.account_name, account.account_id)
            
            # Try to restore previous selection
            if old_selected:
                index = self.combo_rekening.findData(old_selected)
                if index >= 0:
                    self.combo_rekening.setCurrentIndex(index)
        else:
            self.combo_rekening.addItem("Tidak ada rekening tersedia", None)

    # --- Step Flow Handlers ---
    def onKonfirmasiClicked(self):
        target_name = self.input_target_name.text().strip()
        selected_account_id = self.combo_rekening.currentData()
        selected_account_name = self.combo_rekening.currentText()
        nominal_str = self.input_nominal.text().strip()
        deadline_qdate = self.date_deadline.date()
        deadline = datetime(deadline_qdate.year(), deadline_qdate.month(), deadline_qdate.day())
        
        # Validasi
        if not target_name:
            self._show_warning("Silakan masukkan **Nama Target**.")
            return
        
        if not selected_account_id:
            self._show_warning("Silakan **pilih rekening** untuk menabung. Jika tidak ada, tambahkan rekening terlebih dahulu.")
            return

        if not nominal_str or not nominal_str.isdigit() or int(nominal_str) <= 0:
            self._show_warning("Silakan masukkan **Nominal** yang valid (angka lebih dari 0).")
            return
        
        nominal_target = int(nominal_str)
        
        # Store data for saving
        self.target_data = {
            "target_id": str(uuid.uuid4()),
            "account_id": selected_account_id,
            "account_name": selected_account_name,
            "target_name": target_name,
            "target_amount": nominal_target,
            "deadline": deadline,
        }

        conf_dialog = KonfirmasiTargetDialog(target_name, selected_account_name, nominal_target, deadline, self)
        
        # Update dialog title and button if in edit mode
        if self.edit_mode:
            conf_dialog.setWindowTitle("Konfirmasi Edit Target")
            # Find and update the "Buat Target" button to "Simpan"
            for btn in conf_dialog.findChildren(QPushButton):
                if btn.text() == "Buat Target":
                    btn.setText("Simpan")
                    break
        
        parent_rect = self.geometry()
        x = parent_rect.x() + (parent_rect.width() - conf_dialog.width()) // 2
        y = parent_rect.y() + (parent_rect.height() - conf_dialog.height()) // 2
        conf_dialog.move(x, y)

        if conf_dialog.exec() == QDialog.DialogCode.Accepted: 
            self.onTargetConfirmed()
    
    def _show_warning(self, message):
        """Show warning message box."""
        msg = QMessageBox(self)
        msg.setWindowTitle("Peringatan Input")
        msg.setText(message)
        msg.setIcon(QMessageBox.Icon.Warning)
        msg.setStyleSheet("""
            QMessageBox { background-color: white; } 
            QLabel { color: black; } 
            QPushButton { background-color: #5873E8; color: white; border-radius: 5px; padding: 5px 15px; }
        """)
        msg.exec()

    def onTargetConfirmed(self):
        """Save target to database using TargetController."""
        if not hasattr(self, 'target_data') or not self.target_data:
            return
        
        if not self.target_controller:
            return
        
        try:
            if self.edit_mode:
                # Update existing target (name, amount, and deadline)
                success = self.target_controller.update_target(
                    target_id=self.edit_target_id,
                    target_name=self.target_data["target_name"],
                    target_amount=self.target_data["target_amount"],
                    deadline=self.target_data["deadline"]
                )
            else:
                # Create new target
                success = self.target_controller.create_target(
                    target_id=self.target_data["target_id"],
                    account_id=self.target_data["account_id"],
                    target_name=self.target_data["target_name"],
                    target_amount=self.target_data["target_amount"],
                    deadline=self.target_data["deadline"]
                )
            
            if success:
                # Show success dialog
                selesai_dialog = TargetSelesaiDialog(self)
                parent_rect = self.geometry()
                x = parent_rect.x() + (parent_rect.width() - selesai_dialog.width()) // 2
                y = parent_rect.y() + (parent_rect.height() - selesai_dialog.height()) // 2
                selesai_dialog.move(x, y)
                selesai_dialog.show()
                
                # Reset form and emit signal after delay
                QTimer.singleShot(1500, selesai_dialog.close)
                QTimer.singleShot(1500, self.target_added.emit)
                QTimer.singleShot(1600, self.resetUI)
        except Exception as e:
            print(f"Error saving target: {e}")
        
    

    def resetUI(self): 
        """Reset form to initial state."""
        self.input_target_name.clear()
        self.combo_rekening.setCurrentIndex(0)
        self.input_nominal.clear()
        self.date_deadline.setDate(QDate.currentDate().addDays(30))
        self.target_data = None
        self.clear_edit_mode()
    
    def set_edit_mode(self, target_id: str, target_data: dict):
        """Set the form to edit mode with existing target data (only name and amount editable)"""
        self.edit_mode = True
        self.edit_target_id = target_id
        self.edit_target_data = target_data
        
        # Update title
        self.title_label.setText("Edit Target Menabung")
        
        # Populate form with existing data
        target_name = target_data.get('target_name', '')
        target_amount = target_data.get('target_amount', 0)
        account_id = target_data.get('account_id', '')
        
        # Set target name (editable)
        self.input_target_name.setText(target_name)
        
        # Set account dropdown (read-only in edit mode)
        index = self.combo_rekening.findData(account_id)
        if index >= 0:
            self.combo_rekening.setCurrentIndex(index)
        self.combo_rekening.setEnabled(False)  # Prevent changing account
        
        # Set nominal (editable)
        self.input_nominal.setText(str(int(target_amount)))
        
        # Set deadline if available (editable)
        if 'deadline' in target_data and target_data['deadline']:
            deadline = target_data['deadline']
            self.date_deadline.setDate(QDate(deadline.year, deadline.month, deadline.day))
        
        # Update button text
        self.btn_konfirmasi.setText("Simpan Perubahan")
    
    def clear_edit_mode(self):
        """Clear edit mode and reset to add mode"""
        self.edit_mode = False
        self.edit_target_id = None
        self.edit_target_data = None
        
        # Reset title
        self.title_label.setText("Tambah Target Menabung")
        
        # Reset button text
        self.btn_konfirmasi.setText("Konfirmasi")
        
        # Re-enable fields
        self.combo_rekening.setEnabled(True)


# --- C. Contoh Driver Code untuk TambahTargetView (untuk pengujian) ---
if __name__ == '__main__':
    import sys
    from PyQt6.QtWidgets import QApplication

    app = QApplication(sys.argv)
    window = TambahTargetView()
    window.show()
    sys.exit(app.exec())