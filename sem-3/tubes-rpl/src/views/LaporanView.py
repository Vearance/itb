from PyQt6.QtWidgets import (
    QWidget, QLabel, QPushButton, QVBoxLayout, QHBoxLayout, 
    QFrame, QScrollArea, QSizePolicy, QComboBox
)
from PyQt6.QtCore import Qt, pyqtSignal, QSize
from PyQt6.QtGui import QIcon
from models.User import User
from datetime import datetime


class DetailView(QWidget):
    """Frontend untuk Detail Transaksi atau Laporan"""
    
    # Signals
    back_requested = pyqtSignal()
    report_export_requested = pyqtSignal(str, str)  # format: pdf/csv/txt, account_id (or None for all)
    
    def __init__(self, user: User, detail_type: str = "transaction", data: dict = None,
                 account_controller=None, transaction_controller=None, target_controller=None, 
                 report_controller=None):
        """
        Args:
            user: User object
            detail_type: "transaction" untuk transaksi individual, "report" untuk laporan
            data: Dictionary berisi data yang akan ditampilkan
            account_controller: AccountController instance
            transaction_controller: TransactionController instance
            target_controller: TargetController instance
            report_controller: ReportController instance
        """
        super().__init__()
        self.user = user
        self.detail_type = detail_type
        self.data = data or {}
        self.original_data = {}  # Store original unfiltered data
        self.account_controller = account_controller
        self.transaction_controller = transaction_controller
        self.target_controller = target_controller
        self.report_controller = report_controller
        self.initUI()
    
    def initUI(self):
        """Inisialisasi UI"""
        self.setStyleSheet("background-color: white;")
        
        main_layout = QHBoxLayout()
        main_layout.setContentsMargins(0, 0, 0, 0)
        main_layout.setSpacing(0)
        self.setLayout(main_layout)
        
        # Content area
        content_widget = QWidget()
        self.content_layout = QVBoxLayout(content_widget)
        self.content_layout.setContentsMargins(40, 40, 40, 40)
        self.content_layout.setSpacing(25)
        
        # Header dengan tombol back
        header = self.createHeader()
        self.content_layout.addLayout(header)
        
        # Scroll area untuk konten
        self.scroll_area = QScrollArea()
        self.scroll_area.setWidgetResizable(True)
        self.scroll_area.setStyleSheet("""
            QScrollArea { border: none; background-color: white; }
            QScrollBar:vertical { width: 10px; }
        """)
        
        self.scroll_content = QWidget()
        self.scroll_layout = QVBoxLayout(self.scroll_content)
        self.scroll_layout.setContentsMargins(0, 0, 0, 0)
        self.scroll_layout.setSpacing(20)
        
        # Status card
        status_card = self.createStatusCard()
        self.scroll_layout.addWidget(status_card)
        
        # Transaction details / Report details
        if self.detail_type == "transaction":
            detail_card = self.createTransactionDetailCard()
            self.scroll_layout.addWidget(detail_card)
        else:
            # Report: multiple account cards
            account_cards = self.createReportAccountCards()
            for card in account_cards:
                self.scroll_layout.addWidget(card)

        # Export button
        export_button = self.createExportButton()
        self.scroll_layout.addWidget(export_button)
        
        self.scroll_layout.addStretch()
        self.scroll_area.setWidget(self.scroll_content)
        self.content_layout.addWidget(self.scroll_area, 1)
        
        main_layout.addWidget(content_widget)

    def refresh(self):
        """Refresh laporan dengan data terbaru dari backend"""
        if self.detail_type == "report" and self.report_controller:
            self.loadReportData()
    
    def loadReportData(self):
        """Load report data dari ReportController per account"""
        if not self.report_controller or not self.account_controller or not self.transaction_controller:
            return
        
        # Get all accounts for current user
        accounts = self.account_controller.get_user_accounts(self.user.user_id)
        
        # Prepare account data with transaction summaries
        accounts_data = []
        all_transactions = []
        
        for account in accounts:
            # Get transactions for this account
            transactions = self.transaction_controller.get_account_transactions(account.account_id)
            
            # Filter transactions by current month/year (atau bisa dibuat configurable)
            current_month = datetime.now().month
            current_year = datetime.now().year
            filtered_transactions = [
                t for t in transactions 
                if t.date.month == current_month and t.date.year == current_year
            ]
            
            # Generate report for this account (now includes detailed transactions)
            account_report = self.report_controller.generate_transaction_report(
                filtered_transactions,
                account
            )
            
            accounts_data.append(account_report)
            all_transactions.extend(filtered_transactions)
        
        # Sort transactions by date descending
        all_transactions.sort(key=lambda t: t.date, reverse=True)
        
        # Build transactions list from report data (which now includes details)
        transactions_list = []
        for account_data in accounts_data:
            if 'transactions' in account_data:
                for txn in account_data['transactions']:
                    transactions_list.append({
                        "transaction_id": txn.get('transaction_id'),
                        "account_id": account_data.get('account_id'),
                        "date": txn.get('date'),
                        "description": txn.get('description'),
                        "amount": txn.get('amount'),
                        "type": txn.get('type')
                    })
        
        # Store ORIGINAL unfiltered data
        self.original_data = {
            "accounts": accounts_data,
            "filter_type": "Bulanan",
            "start_date": datetime.now().replace(day=1),
            "end_date": datetime.now(),
            "transactions": transactions_list
        }
        
        # Initialize display data with full data
        self.data = {
            "accounts": accounts_data,
            "filter_type": "Bulanan",
            "start_date": datetime.now().replace(day=1),
            "end_date": datetime.now(),
            "transactions": transactions_list
        }
        
        # Populate account combo box
        self.populateAccountCombo(accounts_data)
    
    def populateAccountCombo(self, accounts_data):
        """Populate account combo box with all accounts or filtered account"""
        if not hasattr(self, 'account_combo'):
            return
        
        self.account_combo.blockSignals(True)
        self.account_combo.clear()
        
        # Add placeholder option
        self.account_combo.addItem("--Pilih Account--", None)
        
        # Add individual accounts
        for account_data in accounts_data:
            account_name = account_data.get('account_name', 'Unknown')
            account_id = account_data.get('account_id')
            self.account_combo.addItem(account_name, account_id)
        
        self.account_combo.blockSignals(False)
        self.account_combo.setCurrentIndex(0)  # Default to "--Pilih Account--"
    
    def onAccountChanged(self, text):
        """Handle account selection change"""
        if not hasattr(self, 'account_combo'):
            return
        
        selected_account_id = self.account_combo.currentData()
        
        # Don't filter if placeholder is selected
        if selected_account_id is None:
            return
        
        # Filter from ORIGINAL data to preserve all transactions
        original_accounts = self.original_data.get("accounts", [])
        original_transactions = self.original_data.get("transactions", [])
        
        filtered_accounts = [
            acc for acc in original_accounts
            if acc.get('account_id') == selected_account_id
        ]
        
        # Update self.data with filtered accounts but keep all transactions
        self.data["accounts"] = filtered_accounts
        self.data["transactions"] = original_transactions
        
        # Refresh only the content area, not the entire layout
        self.refreshReportContent()
    
    def refreshReportContent(self):
        """Refresh konten laporan tanpa rebuild layout"""
        # Clear old content from scroll layout
        while self.scroll_layout.count() > 0:
            child = self.scroll_layout.takeAt(0)
            if child.widget():
                child.widget().deleteLater()
        
        # Rebuild report cards
        # Status card
        status_card = self.createStatusCard()
        self.scroll_layout.addWidget(status_card)
        
        # Report cards
        report_cards = self.createReportAccountCards()
        for card in report_cards:
            self.scroll_layout.addWidget(card)
        
        # Export button
        export_button = self.createExportButton()
        self.scroll_layout.addWidget(export_button)
        
        # Add stretch at end
        self.scroll_layout.addStretch()
    
    def _get_account_name(self, account_id: str) -> str:
        """Helper untuk mendapatkan nama account berdasarkan account_id"""
        if not self.account_controller:
            return "-"
        account = self.account_controller.get_account(account_id)
        return account.account_name if account else "-"

    def createHeader(self):
        """Buat header dengan tombol back (hanya untuk detail view)"""
        layout = QHBoxLayout()
        layout.setSpacing(15)
        
        # Back button - hanya tampil untuk detail_type transaction
        if self.detail_type == "transaction":
            btn_back = QPushButton()
            btn_back.setFixedSize(40, 40)
            btn_back.setCursor(Qt.CursorShape.PointingHandCursor)
            btn_back.setStyleSheet("""
                QPushButton {
                    background-color: transparent;
                    border: none;
                    font-size: 24px;
                }
                QPushButton:hover { background-color: #F0F2FF; border-radius: 8px; }
            """)
            btn_back.setText("←")
            btn_back.clicked.connect(self.onBackClicked)
            layout.addWidget(btn_back)
        
        # Title
        if self.detail_type == "transaction":
            title_text = "Detail Transaksi"
        else:
            title_text = "Laporan Keuangan"
        
        title = QLabel(title_text)
        title.setStyleSheet("font-size: 24px; color: black; font-weight: bold; border: none;")
        
        layout.addWidget(title)
        layout.addStretch()
        
        # Account selector - hanya untuk report view
        if self.detail_type == "report":
            self.account_combo = QComboBox()
            self.account_combo.setStyleSheet("""
                QComboBox {
                    background-color: white;
                    border: 1px solid #DCDCDC;
                    border-radius: 8px;
                    padding: 8px 12px;
                    font-size: 13px;
                    min-width: 200px;
                    color: #333333;
                }
                QComboBox:hover { border: 1px solid #5873E8; }
                QComboBox::drop-down { border: none; }
                QComboBox QAbstractItemView {
                    background-color: white;
                    color: #333333;
                    selection-background-color: #5873E8;
                }
            """)
            self.account_combo.currentTextChanged.connect(self.onAccountChanged)
            layout.addWidget(self.account_combo)
        
        return layout
    
    def createStatusCard(self):
        """Buat card status transaksi"""
        card = QFrame()
        card.setStyleSheet("""
            QFrame {
                background-color: #E8ECFC;
                border-radius: 15px;
            }
        """)
        card.setFixedHeight(100)
        
        layout = QHBoxLayout(card)
        layout.setContentsMargins(30, 20, 30, 20)
        layout.setSpacing(0)
        
        # Left side
        left_layout = QVBoxLayout()
        left_layout.setSpacing(5)
        
        # Judul 'Keterangan Transaksi' diubah menjadi 'Periode Laporan' jika mode report
        if self.detail_type == "report":
            title_label = QLabel("Periode Laporan")
        else:
            title_label = QLabel("Keterangan Transaksi")
            
        title_label.setStyleSheet("font-size: 16px; font-weight: bold; color: black; border: none;")
        
        # Get description based on type
        if self.detail_type == "transaction":
            trans_type = self.data.get("type", "income")
            desc = "Pendapatan" if trans_type == "income" else "Pengeluaran"
            date_str = self.data.get("date", datetime.now()).strftime("%A, %d %B %Y")
            desc_text = f"{desc} - {date_str}"
        else:
            # Report type
            filter_type = self.data.get("filter_type", "Bulanan")
            desc_text = self.getReportDateLabel(filter_type)
        
        desc_label = QLabel(desc_text)
        desc_label.setStyleSheet("font-size: 13px; color: #606060; border: none;")
        
        left_layout.addWidget(title_label)
        left_layout.addWidget(desc_label)
        
        # Right side - status
        if self.detail_type == "report":
            filter_type = self.data.get("filter_type", "Bulanan")
            if filter_type == "Pilih Tanggal":
                status_text = "LAPORAN CUSTOM"
            else:
                status_text = filter_type.upper()
            status_color = "#5873E8"
        else:
            status_color = "#2ECC71" if self.data.get("type") == "income" else "#E74C3C"
            status_text = "SUKSES" if self.data.get("status", True) else "GAGAL"
        
        status_label = QLabel(status_text)
        status_label.setStyleSheet(f"font-size: 14px; font-weight: bold; color: {status_color}; border: none;")
        status_label.setAlignment(Qt.AlignmentFlag.AlignRight | Qt.AlignmentFlag.AlignVCenter)
        
        layout.addLayout(left_layout)
        layout.addStretch()
        layout.addWidget(status_label)
        
        return card
    
    def getReportDateLabel(self, filter_type):
        """Get formatted date label for report"""
        start = self.data.get("start_date", datetime.now())
        end = self.data.get("end_date", datetime.now())
        
        if filter_type == "Harian":
            return start.strftime("%A, %d %B %Y")
        elif filter_type == "Mingguan":
            return f"{start.strftime('%d %B')} - {end.strftime('%d %B %Y')}"
        elif filter_type == "Bulanan":
            return start.strftime("%B %Y")
        elif filter_type == "Pilih Tanggal":
            if start.month == end.month:
                return f"{start.strftime('%d')} - {end.strftime('%d %B %Y')}"
            else:
                return f"{start.strftime('%d %B')} - {end.strftime('%d %B %Y')}"
        return ""
    
    def createTransactionDetailCard(self):
        """Buat card detail untuk transaksi individual"""
        card = QFrame()
        card.setStyleSheet("""
            QFrame {
                background-color: #E8ECFC;
                border-radius: 15px;
            }
        """)
        
        layout = QVBoxLayout(card)
        layout.setContentsMargins(30, 25, 30, 25)
        layout.setSpacing(15)
        
        # Title
        title = QLabel(self.data.get("account_name", "Nama Target yg dipilih"))
        title.setStyleSheet("font-size: 17px; font-weight: bold; color: black; border: none;")
        layout.addWidget(title)
        
        # Details
        details = [
            ("No. rek", self.data.get("account_id", "82331241249174123")),
            ("Nominal Total", f"Rp{self.data.get('amount', 0):,.0f}"),
            ("Biaya Admin", f"Rp{self.data.get('admin_fee', 0):,.0f}"),
        ]
        
        for label, value in details:
            row = self.createDetailRow(label, value, is_total=False)
            layout.addWidget(row)
        
        # Total
        total_row = self.createDetailRow(
            "Total transaksi dari tanggal\nyang dipilih",
            f"Rp{self.data.get('total', 0):,.0f}",
            is_total=True
        )
        layout.addWidget(total_row)
        
        return card
    
    def createReportAccountCards(self):
        """Buat multiple cards untuk laporan (per account dengan detail transaksi)"""
        cards = []
        accounts = self.data.get("accounts", [])
        
        # Handle jika data accounts kosong
        if not accounts:
            empty_lbl = QLabel("Tidak ada data akun pada periode ini.")
            empty_lbl.setStyleSheet("color: grey; font-style: italic;")
            cards.append(empty_lbl)
            return cards
        
        for account in accounts:
            # Account summary card
            summary_card = QFrame()
            summary_card.setStyleSheet("""
                QFrame {
                    background-color: #E8ECFC;
                    border-radius: 15px;
                }
            """)
            
            summary_layout = QVBoxLayout(summary_card)
            summary_layout.setContentsMargins(30, 25, 30, 25)
            summary_layout.setSpacing(15)
            
            # Title
            title = QLabel(account.get("account_name", "Nama Akun"))
            title.setStyleSheet("font-size: 17px; font-weight: bold; color: black; border: none;")
            summary_layout.addWidget(title)
            
            # Summary details
            details = [
                ("No. Rek", account.get("account_id", "-")),
                ("Saldo Awal", f"Rp {account.get('current_balance', 0):,.0f}"),
                ("Total Masuk", f"Rp {account.get('total_income', 0):,.0f}"),
                ("Total Keluar", f"Rp {account.get('total_expense', 0):,.0f}"),
            ]
            
            for label, value in details:
                row = self.createDetailRow(label, value, is_total=False)
                summary_layout.addWidget(row)
            
            # Net change
            net_change = account.get('net_change', 0)
            net_change_color = "#2ecc71" if net_change >= 0 else "#e74c3c"
            net_row = self.createDetailRow(
                "Perubahan Bersih",
                f"Rp {net_change:,.0f}",
                is_total=False
            )
            net_row.findChild(QLabel).setStyleSheet(f"font-size: 13px; font-weight: bold; color: {net_change_color}; border: none;")
            summary_layout.addWidget(net_row)
            
            cards.append(summary_card)
            
            # Transaction details card
            transactions = self.data.get("transactions", [])
            account_transactions = [t for t in transactions if t.get("account_id") == account.get("account_id")]
            
            if account_transactions:
                transaction_card = QFrame()
                transaction_card.setStyleSheet("""
                    QFrame {
                        background-color: white;
                        border: 1px solid #DCDCDC;
                        border-radius: 15px;
                    }
                """)
                
                trans_layout = QVBoxLayout(transaction_card)
                trans_layout.setContentsMargins(30, 25, 30, 25)
                trans_layout.setSpacing(15)
                
                # Header
                trans_header = QLabel("Detail Transaksi")
                trans_header.setStyleSheet("font-size: 16px; font-weight: bold; color: black; border: none;")
                trans_layout.addWidget(trans_header)
                
                # Transaction rows
                for idx, trans in enumerate(account_transactions):
                    trans_row = self.createTransactionDetailRow(trans)
                    trans_layout.addWidget(trans_row)
                    
                    # Add separator except for last item
                    if idx < len(account_transactions) - 1:
                        sep = QFrame()
                        sep.setStyleSheet("background-color: #EBEBEB; border: none;")
                        sep.setFixedHeight(1)
                        trans_layout.addWidget(sep)
                
                cards.append(transaction_card)
        
        return cards
    
    def createTransactionDetailRow(self, transaction: dict) -> QFrame:
        """Buat baris untuk satu transaksi"""
        row = QFrame()
        row.setStyleSheet("border: none; background: transparent;")
        
        layout = QHBoxLayout(row)
        layout.setContentsMargins(0, 10, 0, 10)
        layout.setSpacing(15)
        
        # Left side: Description and date
        left_layout = QVBoxLayout()
        left_layout.setContentsMargins(0, 0, 0, 0)
        left_layout.setSpacing(5)
        
        desc = QLabel(transaction.get("description", ""))
        desc.setStyleSheet("font-size: 13px; color: black; font-weight: 500; border: none;")
        left_layout.addWidget(desc)
        
        date = QLabel(transaction.get("date", ""))
        date.setStyleSheet("font-size: 12px; color: #909090; border: none;")
        left_layout.addWidget(date)
        
        # Right side: Amount with type indicator
        amount_text = f"Rp {transaction.get('amount', 0):,.0f}"
        trans_type = transaction.get("type", "").lower()
        amount_color = "#2ecc71" if trans_type == "income" else "#e74c3c"
        amount_sign = "+" if trans_type == "income" else "-"
        
        amount = QLabel(f"{amount_sign}{amount_text}")
        amount.setStyleSheet(f"font-size: 13px; font-weight: bold; color: {amount_color}; border: none;")
        amount.setAlignment(Qt.AlignmentFlag.AlignRight)
        
        layout.addLayout(left_layout, 1)
        layout.addWidget(amount)
        
        return row
    
    def createDetailRow(self, label: str, value: str, is_total: bool = False):
        """Helper untuk membuat baris detail"""
        row = QFrame()
        row.setStyleSheet("border: none; background: transparent;")
        
        layout = QHBoxLayout(row)
        layout.setContentsMargins(0, 5, 0, 5)
        layout.setSpacing(0)
        
        label_widget = QLabel(label)
        value_widget = QLabel(value)
        
        if is_total:
            label_widget.setStyleSheet("font-size: 14px; font-weight: bold; color: black; border: none;")
            value_widget.setStyleSheet("font-size: 16px; font-weight: bold; color: black; border: none;")
        else:
            label_widget.setStyleSheet("font-size: 13px; color: #606060; border: none;")
            value_widget.setStyleSheet("font-size: 13px; color: #606060; border: none;")
        
        value_widget.setAlignment(Qt.AlignmentFlag.AlignRight)
        
        layout.addWidget(label_widget)
        layout.addStretch()
        layout.addWidget(value_widget)
        
        return row
    
    def createExportButton(self):
        """Buat tombol export laporan"""
        btn = QPushButton("Buat Laporan Keuangan")
        btn.setFixedHeight(55)
        btn.setCursor(Qt.CursorShape.PointingHandCursor)
        btn.setStyleSheet("""
            QPushButton {
                background-color: white;
                color: #5873E8;
                border: 2px solid #5873E8;
                border-radius: 12px;
                font-weight: bold;
                font-size: 15px;
            }
            QPushButton:hover { background-color: #F0F2FF; }
        """)
        btn.clicked.connect(self.onExportClicked)
        return btn
    
    def onBackClicked(self):
        """Handle klik tombol back"""
        self.back_requested.emit()
        # print("Back to history")
    
    def onExportClicked(self):
        """Handle klik tombol export"""
        # Get selected account_id from combo box (None if "Semua Akun" selected)
        selected_account_id = None
        if hasattr(self, 'account_combo'):
            selected_account_id = self.account_combo.currentData()
        
        self.report_export_requested.emit("pdf", selected_account_id if selected_account_id else None)
        # print("Export report as PDF")
