from datetime import datetime
import os
from PyQt6.QtWidgets import (
    QMainWindow, QWidget, QVBoxLayout, QHBoxLayout, QStackedWidget, 
    QFrame, QPushButton, QSizePolicy, QMessageBox
)
from PyQt6.QtCore import Qt, pyqtSignal, QSize
from PyQt6.QtGui import QIcon

from views.LoginView import LoginView
from views.Homepage import Homepage
from views.RekeningView import RekeningView
from views.HistoryView import HistoryView
from views.TargetView import ProgressView
from views.TambahRekening import TambahRekeningView
from views.TambahTransaksi import TransaksiView
from views.TambahTarget import TambahTargetView
from views.LaporanView import DetailView

class Sidebar(QFrame):
    page_changed = pyqtSignal(int) # Emits index of page to switch to

    def __init__(self):
        super().__init__()
        self.setObjectName("navMenu")
        self.setFixedWidth(110)
        self.setSizePolicy(QSizePolicy.Policy.Fixed, QSizePolicy.Policy.Expanding)
        self.setStyleSheet("background-color: white;")
        
        layout = QVBoxLayout(self)
        layout.setContentsMargins(20, 40, 20, 40)
        layout.setSpacing(25)
        
        self.buttons = []
        self.menu_items = [
            ("Home", "Home", 0),      
            ("Tabungan", "Tabungan", 1), 
            ("Riwayat", "Riwayat", 2),
            ("Statistik", "Progress", 3),
            ("Laporan", "Laporan", 4),
        ]
        
        for label, icon_name, index in self.menu_items:
            btn = QPushButton()
            btn.setFixedSize(65, 65) 
            btn.setCursor(Qt.CursorShape.PointingHandCursor)
            btn.setToolTip(label)
            btn.setProperty("index", index)
            btn.setProperty("icon_name", icon_name)
            
            btn.clicked.connect(lambda checked, idx=index: self.handle_click(idx))
            
            layout.addWidget(btn, 0, Qt.AlignmentFlag.AlignCenter)
            self.buttons.append(btn)
            
        layout.addStretch()
        
        self.set_active_index(0)

    def handle_click(self, index):
        self.set_active_index(index)
        self.page_changed.emit(index)

    def set_active_index(self, index):
        for btn in self.buttons:
            idx = btn.property("index")
            icon_name = btn.property("icon_name")
            
            if idx == index:
                icon_path = f"img/{icon_name}Selected.svg"
                btn.setStyleSheet("background-color: transparent; border: none;")
                btn.setIconSize(QSize(64, 64))
            else:
                icon_path = f"img/{icon_name}.svg"
                btn.setStyleSheet("""
                    QPushButton { background-color: transparent; border-radius: 15px; }
                    QPushButton:hover { background-color: #F0F2FF; }
                """)
                btn.setIconSize(QSize(42, 42))
            
            btn.setIcon(QIcon(icon_path))

class MainContainer(QWidget):
    def __init__(self, user, account_controller, transaction_controller, target_controller, report_controller):
        super().__init__()
        
        self.user = user
        self.account_controller = account_controller
        self.transaction_controller = transaction_controller
        self.target_controller = target_controller
        self.report_controller = report_controller
        
        layout = QHBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(0)
        
        self.sidebar = Sidebar()
        self.sidebar.page_changed.connect(self.switch_page)
        
        self.content_stack = QStackedWidget()
        
        # Initialize Views
        # Index 0: Home
        self.homepage = Homepage(user, account_controller, transaction_controller, target_controller, report_controller)
        # Index 1: Tabungan (Rekening)
        self.rekening_view = RekeningView(user, account_controller, transaction_controller, target_controller)
        # Index 2: Riwayat
        self.history_view = HistoryView(user, account_controller, transaction_controller)
        # Index 3: Statistik/Progress (Target) - now includes transaction_controller for chart
        self.target_view = ProgressView(user, account_controller, target_controller, transaction_controller)
        # Index 4: Laporan
        self.laporan_view = DetailView(user, "report", {}, account_controller, transaction_controller, target_controller, report_controller)
        # Index 5: Tambah Rekening
        self.tambah_rekening_view = TambahRekeningView(user, account_controller)
        # Index 6: Tambah Transaksi
        self.transaksi_view = TransaksiView(user, account_controller, transaction_controller)
        # Index 7: Tambah Target
        self.tambah_target_view = TambahTargetView(user, account_controller, target_controller)
        
        self.content_stack.addWidget(self.homepage)
        self.content_stack.addWidget(self.rekening_view)
        self.content_stack.addWidget(self.history_view)
        self.content_stack.addWidget(self.target_view)
        self.content_stack.addWidget(self.laporan_view)
        self.content_stack.addWidget(self.tambah_rekening_view)
        self.content_stack.addWidget(self.transaksi_view)
        self.content_stack.addWidget(self.tambah_target_view)
        
        layout.addWidget(self.sidebar)
        layout.addWidget(self.content_stack)
        
        # Connect signals
        self.rekening_view.add_account_requested.connect(self.show_add_account)
        self.rekening_view.transaction_requested.connect(self.show_transaction_view)
        self.rekening_view.account_edit_requested.connect(self.show_edit_account)
        self.rekening_view.account_deleted.connect(self.handle_account_deleted)
        self.tambah_rekening_view.rekening_added.connect(self.handle_account_added)
        self.transaksi_view.transaction_completed.connect(self.handle_transaction_completed)
        self.transaksi_view.back_to_previous.connect(self.show_rekening_view)
        
        # Connect history view signals for transaction editing
        self.history_view.transaction_edit_requested.connect(self.show_edit_transaction)
        
        # Connect target view signals
        self.target_view.add_target_requested.connect(self.show_add_target)
        self.target_view.target_edit_requested.connect(self.show_edit_target)
        self.target_view.target_deleted.connect(self.handle_target_deleted)
        self.tambah_target_view.target_added.connect(self.handle_target_added)
        self.tambah_target_view.back_to_previous.connect(self.handle_target_back)
        
        # Connect laporan (report) view signals
        self.laporan_view.report_export_requested.connect(self.handle_report_export)

    def switch_page(self, index):
        self.content_stack.setCurrentIndex(index)
        
        # Refresh Laporan when switching to it
        if index == 4:
            self.laporan_view.refresh()
        
    def show_add_account(self):
        # Clear any edit mode first
        self.tambah_rekening_view.clear_edit_mode()
        # Switch to TambahRekeningView (Index 5)
        self.content_stack.setCurrentIndex(5)
    
    def show_edit_account(self, account_id: str, account_data: dict):
        """Handle double-click on account to open edit form"""
        # Set edit mode in TambahRekeningView
        self.tambah_rekening_view.set_edit_mode(account_id, account_data)
        # Navigate to TambahRekeningView (Index 5)
        self.content_stack.setCurrentIndex(5)
        
    def handle_account_added(self):
        # Check if we were in edit mode
        was_editing = self.tambah_rekening_view.edit_mode
        
        # Refresh RekeningView
        self.rekening_view.refresh()
        # Also refresh Homepage to update total balance
        self.homepage.refresh()
        # Refresh TargetView to update progress
        self.target_view.refresh()
        # Refresh TransaksiView to update rekening list
        self.transaksi_view.refresh()
        # Switch back to RekeningView (Index 1)
        self.content_stack.setCurrentIndex(1)
        # Update sidebar to reflect active tab
        self.sidebar.set_active_index(1)
    
    def handle_account_deleted(self):
        """Handle account deletion - refresh all relevant views"""
        # Refresh RekeningView
        self.rekening_view.refresh()
        # Also refresh Homepage to update total balance
        self.homepage.refresh()
        # Refresh TargetView to update progress
        self.target_view.refresh()
        # Refresh TransaksiView to update rekening list
        self.transaksi_view.refresh()
        # Refresh HistoryView in case deleted account's transactions were shown
        self.history_view.refresh()
    
    def show_transaction_view(self, account_id):
        """Handle transaction button click"""
        # Clear any edit mode first
        self.transaksi_view.clear_edit_mode()
        # Navigate to TransaksiView (Index 6)
        self.content_stack.setCurrentIndex(6)
        # TODO: Pass account_id to TransaksiView if needed
    
    def show_rekening_view(self):
        """Navigate back to RekeningView (Index 1)"""
        self.content_stack.setCurrentIndex(1)
    
    def show_edit_transaction(self, transaction_id: str, transaction_data: dict):
        """Handle double-click on transaction to open edit form"""
        # Set edit mode in TransaksiView
        self.transaksi_view.set_edit_mode(transaction_id, transaction_data)
        # Navigate to TransaksiView (Index 6)
        self.content_stack.setCurrentIndex(6)
    
    def handle_transaction_completed(self):
        """Handle transaction completion - refresh all relevant views and switch back"""
        # Check if we were in edit mode (came from history view)
        was_editing = self.transaksi_view.edit_mode
        
        # Refresh RekeningView to show updated balances
        self.rekening_view.refresh()
        # Refresh Homepage to update total balance
        self.homepage.refresh()
        # Refresh HistoryView to show new/updated transaction
        self.history_view.refresh()
        # Refresh TargetView since progress is based on account balance
        self.target_view.refresh()
        
        if was_editing:
            # If we were editing from history, go back to history view
            self.content_stack.setCurrentIndex(2)
            self.sidebar.set_active_index(2)
        else:
            # Switch back to RekeningView (Index 1)
            self.content_stack.setCurrentIndex(1)
            # Update sidebar to reflect active tab
            self.sidebar.set_active_index(1)
    
    def show_add_target(self):
        """Navigate to TambahTargetView (Index 7)"""
        self.tambah_target_view.clear_edit_mode()
        self.content_stack.setCurrentIndex(7)
    
    def show_edit_target(self, target_id: str, target_data: dict):
        """Handle double-click on target to open edit form"""
        # Set edit mode in TambahTargetView
        self.tambah_target_view.set_edit_mode(target_id, target_data)
        # Navigate to TambahTargetView (Index 7)
        self.content_stack.setCurrentIndex(7)
    
    def handle_target_added(self):
        """Handle target added - refresh target view and switch back"""
        # Refresh ProgressView to show new target
        self.target_view.refresh()
        # Also refresh Homepage to update progress display
        self.homepage.refresh()
        # Switch back to ProgressView (Index 3)
        self.content_stack.setCurrentIndex(3)
        # Update sidebar to reflect active tab
        self.sidebar.set_active_index(3)
    
    def handle_target_deleted(self):
        """Handle target deleted - refresh all relevant views"""
        # Refresh ProgressView to remove deleted target
        self.target_view.refresh()
        # Also refresh Homepage to update progress display
        self.homepage.refresh()
        # No need to change page, user stays on TargetView
    
    def handle_target_back(self):
        """Handle back button from TambahTargetView"""
        # Switch back to ProgressView (Index 3)
        self.content_stack.setCurrentIndex(3)
        # Update sidebar to reflect active tab
        self.sidebar.set_active_index(3)
    
    def handle_report_export(self, export_format, selected_account_id=None):
        """Handle report export request"""
        # Get user accounts
        accounts = self.account_controller.get_user_accounts(self.user.user_id)
        if not accounts:
            return
        
        # Filter accounts based on selection
        if selected_account_id is None:
            # Export all accounts
            accounts_to_export = accounts
        else:
            # Export only selected account
            accounts_to_export = [acc for acc in accounts if acc.account_id == selected_account_id]
        
        if not accounts_to_export:
            return
        
        current_account = accounts_to_export[0]
        
        # Get transactions for current account
        transactions = self.transaction_controller.get_account_transactions(current_account.account_id)
        
        # Generate report data
        report_data = self.report_controller.generate_transaction_report(transactions, current_account)
        
        # Determine file format and extension        
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        output_dir = "output"
        
        if export_format.lower() == 'pdf':
            filename = os.path.join(output_dir, f"laporan_{current_account.account_name}_{timestamp}.pdf")
            format_type = 'pdf'
        elif export_format.lower() == 'csv':
            filename = os.path.join(output_dir, f"laporan_{current_account.account_name}_{timestamp}.csv")
            format_type = 'csv'
        else:
            filename = os.path.join(output_dir, f"laporan_{current_account.account_name}_{timestamp}.txt")
            format_type = 'txt'
        
        # Export the report
        success = self.report_controller.export_report(report_data, filename, format_type)
        
        # Show result message
        if success:
            QMessageBox.information(
                self, 
                "Export Berhasil", 
                f"Laporan berhasil diekspor ke:\n{filename}\n\n"
                f"Total Transaksi: {report_data.get('transaction_count', 0)}\n"
                f"Total Masuk: Rp {report_data.get('total_income', 0):,.0f}\n"
                f"Total Keluar: Rp {report_data.get('total_expense', 0):,.0f}"
            )
        else:
            QMessageBox.warning(
                self, 
                "Export Gagal", 
                "Gagal mengekspor laporan. Silakan coba lagi."
            )


class MainWindow(QMainWindow):
    def __init__(self, login_controller, account_controller, transaction_controller, target_controller, report_controller):
        super().__init__()
        self.setWindowTitle("Tabungin")
        self.setFixedSize(1000, 700)
        
        self.login_controller = login_controller
        self.account_controller = account_controller
        self.transaction_controller = transaction_controller
        self.target_controller = target_controller
        self.report_controller = report_controller
        
        self.main_stack = QStackedWidget()
        self.setCentralWidget(self.main_stack)
        
        self.login_view = LoginView(
            login_controller, account_controller, transaction_controller, target_controller, report_controller
        )
        self.login_view.login_success.connect(self.handle_login_success)
        
        self.main_stack.addWidget(self.login_view)
        
    def handle_login_success(self, user):
        self.main_container = MainContainer(
            user, 
            self.account_controller, 
            self.transaction_controller, 
            self.target_controller, 
            self.report_controller
        )
        self.main_stack.addWidget(self.main_container)
        self.main_stack.setCurrentWidget(self.main_container)
