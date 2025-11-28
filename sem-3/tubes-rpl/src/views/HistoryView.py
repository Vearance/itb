from PyQt6.QtWidgets import (
    QWidget, QLabel, QPushButton, QVBoxLayout, QHBoxLayout, 
    QFrame, QScrollArea, QSizePolicy, QLineEdit, QDialog
)
from PyQt6.QtCore import Qt, pyqtSignal, QSize, QDate
from PyQt6.QtGui import QIcon
from models.User import User
from datetime import datetime, timedelta
from collections import defaultdict


class DateRangeDialog(QDialog):
    """Dialog untuk memilih date range dengan input manual"""
    
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setWindowTitle("Pilih Tanggal")
        self.setFixedSize(600, 380)
        self.start_date = None
        self.end_date = None
        self.initUI()
    
    def initUI(self):
        layout = QVBoxLayout(self)
        layout.setSpacing(25)
        layout.setContentsMargins(40, 40, 40, 40)
        
        # Instruction label
        instruction = QLabel("Format: DD/MM/YYYY (contoh: 24/11/2025)")
        instruction.setStyleSheet("color: #808080; font-size: 13px;")
        layout.addWidget(instruction)
        
        layout.addSpacing(10)
        
        # Start date section
        start_label = QLabel("Tanggal Mulai:")
        start_label.setStyleSheet("font-weight: bold; font-size: 15px; color: black;")
        layout.addWidget(start_label)
        
        self.start_date_input = QLineEdit()
        self.start_date_input.setPlaceholderText("DD/MM/YYYY")
        self.start_date_input.setFixedHeight(50)
        self.start_date_input.setStyleSheet("""
            QLineEdit {
                border: 2px solid #5873E8;
                border-radius: 10px;
                padding: 12px;
                font-size: 15px;
                color: black;
                background-color: white;
            }
            QLineEdit:focus {
                border: 2px solid #4A63D0;
            }
        """)
        today = datetime.now()
        self.start_date_input.setText(today.strftime("%d/%m/%Y"))
        layout.addWidget(self.start_date_input)
        
        layout.addSpacing(15)
        
        # End date section
        end_label = QLabel("Tanggal Akhir:")
        end_label.setStyleSheet("font-weight: bold; font-size: 15px; color: black;")
        layout.addWidget(end_label)
        
        self.end_date_input = QLineEdit()
        self.end_date_input.setPlaceholderText("DD/MM/YYYY")
        self.end_date_input.setFixedHeight(50)
        self.end_date_input.setStyleSheet("""
            QLineEdit {
                border: 2px solid #5873E8;
                border-radius: 10px;
                padding: 12px;
                font-size: 15px;
                color: black;
                background-color: white;
            }
            QLineEdit:focus {
                border: 2px solid #4A63D0;
            }
        """)
        self.end_date_input.setText(today.strftime("%d/%m/%Y"))
        layout.addWidget(self.end_date_input)
        
        # Add spacing before buttons
        layout.addSpacing(40)
        
        # Buttons
        buttons_layout = QHBoxLayout()
        buttons_layout.setSpacing(15)
        
        btn_cancel = QPushButton("Batal")
        btn_cancel.setFixedSize(160, 50)
        btn_cancel.setCursor(Qt.CursorShape.PointingHandCursor)
        btn_cancel.setStyleSheet("""
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
        btn_cancel.clicked.connect(self.reject)
        
        btn_confirm = QPushButton("Pilih")
        btn_confirm.setFixedSize(160, 50)
        btn_confirm.setCursor(Qt.CursorShape.PointingHandCursor)
        btn_confirm.setStyleSheet("""
            QPushButton {
                background-color: #5873E8;
                color: white;
                border: none;
                border-radius: 12px;
                font-weight: bold;
                font-size: 15px;
            }
            QPushButton:hover { background-color: #4A63D0; }
        """)
        btn_confirm.clicked.connect(self.validateAndAccept)
        
        buttons_layout.addStretch()
        buttons_layout.addWidget(btn_cancel)
        buttons_layout.addWidget(btn_confirm)
        layout.addLayout(buttons_layout)
    
    def validateAndAccept(self):
        """Validate input dates and accept if valid"""
        try:
            start_text = self.start_date_input.text().strip()
            end_text = self.end_date_input.text().strip()
            
            # Parse dates
            self.start_date = datetime.strptime(start_text, "%d/%m/%Y")
            self.end_date = datetime.strptime(end_text, "%d/%m/%Y")
            
            # Validate: start <= end
            if self.start_date > self.end_date:
                self.start_date_input.setStyleSheet("""
                    QLineEdit {
                        border: 2px solid #E74C3C;
                        border-radius: 8px;
                        padding: 10px;
                        font-size: 14px;
                        color: black;
                        background-color: white;
                    }
                """)
                self.end_date_input.setStyleSheet("""
                    QLineEdit {
                        border: 2px solid #E74C3C;
                        border-radius: 8px;
                        padding: 10px;
                        font-size: 14px;
                        color: black;
                        background-color: white;
                    }
                """)
                return
            
            self.accept()
        except ValueError:
            # Invalid format
            self.start_date_input.setStyleSheet("""
                QLineEdit {
                    border: 2px solid #E74C3C;
                    border-radius: 8px;
                    padding: 10px;
                    font-size: 14px;
                    color: black;
                    background-color: white;
                }
            """)
            self.end_date_input.setStyleSheet("""
                QLineEdit {
                    border: 2px solid #E74C3C;
                    border-radius: 8px;
                    padding: 10px;
                    font-size: 14px;
                    color: black;
                    background-color: white;
                }
            """)
    
    def getDateRange(self):
        """Return (start_date, end_date) as datetime objects"""
        return (self.start_date, self.end_date)


class HistoryView(QWidget):
    """Frontend untuk halaman Riwayat Transaksi"""
    
    # Signals
    transaction_clicked = pyqtSignal(str)  # transaction_id
    transaction_edit_requested = pyqtSignal(str, dict)  # transaction_id, transaction_data (for double-click edit)
    report_requested = pyqtSignal(str, datetime, datetime)  # filter_type, start_date, end_date
    
    def __init__(self, user: User, account_controller=None, transaction_controller=None):
        super().__init__()
        self.user = user
        self.account_controller = account_controller
        self.transaction_controller = transaction_controller
        self.transactions_data = []
        self.current_filter = "Bulanan"  # Default filter
        self.current_start_date = None
        self.current_end_date = None
        self.initUI()
        self.load_transactions()
    
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
        self.content_layout.addLayout(header)
        
        # Filter buttons
        filter_buttons = self.createFilterButtons()
        self.content_layout.addLayout(filter_buttons)
        
        # Summary cards
        self.summary_section = self.createSummarySection()
        self.content_layout.addWidget(self.summary_section)
        
        # Transaction list
        transaction_section = self.createTransactionSection()
        self.content_layout.addWidget(transaction_section, 1)
        
        scroll_area.setWidget(content_widget)
        main_layout.addWidget(scroll_area)

    def refresh(self):
        """Refresh transaction data and update UI"""
        # Reload transactions from database
        self.load_transactions()
        
        # Clear and rebuild the content layout
        while self.content_layout.count():
            child = self.content_layout.takeAt(0)
            if child.widget():
                child.widget().deleteLater()
            elif child.layout():
                # Clear layout items
                while child.layout().count():
                    item = child.layout().takeAt(0)
                    if item.widget():
                        item.widget().deleteLater()
        
        # Rebuild UI components
        # Header
        header = self.createHeader()
        self.content_layout.addLayout(header)
        
        # Filter buttons
        filter_buttons = self.createFilterButtons()
        self.content_layout.addLayout(filter_buttons)
        
        # Summary cards
        self.summary_section = self.createSummarySection()
        self.content_layout.addWidget(self.summary_section)
        
        # Transaction list
        transaction_section = self.createTransactionSection()
        self.content_layout.addWidget(transaction_section, 1)

    def createHeader(self):
        """Buat header"""
        layout = QHBoxLayout()
        
        title = QLabel("Riwayat Transaksi")
        title.setStyleSheet("font-size: 24px; color: black; font-weight: bold; border: none;")
        
        layout.addWidget(title)
        layout.addStretch()
        
        return layout
    
    def createFilterButtons(self):
        """Buat filter buttons"""
        layout = QHBoxLayout()
        layout.setSpacing(12)
        layout.setContentsMargins(0, 0, 0, 0)
        
        filters = ["Harian", "Mingguan", "Bulanan", "Pilih Tanggal"]
        
        for filter_name in filters:
            btn = QPushButton(filter_name)
            btn.setObjectName(f"filter_{filter_name}")
            btn.setFixedSize(140, 40)
            btn.setCursor(Qt.CursorShape.PointingHandCursor)
            
            if filter_name == "Bulanan":  # Default active
                btn.setStyleSheet("""
                    QPushButton {
                        background-color: #5873E8;
                        color: white;
                        border-radius: 12px;
                        font-weight: bold;
                        font-size: 13px;
                    }
                    QPushButton:hover { background-color: #4A63D0; }
                """)
            else:
                btn.setStyleSheet("""
                    QPushButton {
                        background-color: white;
                        color: #5873E8;
                        border: 2px solid #5873E8;
                        border-radius: 12px;
                        font-weight: bold;
                        font-size: 13px;
                    }
                    QPushButton:hover { background-color: #F0F2FF; }
                """)
            
            btn.clicked.connect(lambda checked, name=filter_name: self.onFilterClicked(name))
            layout.addWidget(btn)
        
        layout.addStretch()
        return layout
    
    def createSummarySection(self):
        """Buat section untuk summary cards"""
        section = QFrame()
        section.setStyleSheet("background-color: white; border: none;")
        section.setFixedHeight(120)
        
        layout = QHBoxLayout(section)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(15)
        
        # Get real transaction summary
        summary = self._get_transaction_summary()
        income = summary.get("income", 0)
        expense = summary.get("expense", 0)
        savings = summary.get("savings", 0)
        count = summary.get("count", 0)
        
        # Total Pendapatan
        self.income_card = self.createSummaryCard("Pendapatan", f"Rp{income:,.0f}".replace(",", "."), "#2ECC71")
        layout.addWidget(self.income_card)
        
        # Total Pengeluaran
        self.expense_card = self.createSummaryCard("Pengeluaran", f"Rp{expense:,.0f}".replace(",", "."), "#E74C3C")
        layout.addWidget(self.expense_card)
        
        # Tabungan
        self.savings_card = self.createSummaryCard("Tabungan", f"Rp{savings:,.0f}".replace(",", "."), "#5873E8")
        layout.addWidget(self.savings_card)
        
        # Pembayaran
        self.payment_card = self.createSummaryCard("Jumlah Transaksi", str(count), "#808080")
        layout.addWidget(self.payment_card)
        
        return section
    
    def createSummaryCard(self, title: str, amount: str, color: str):
        """Buat summary card individual"""
        card = QFrame()
        card.setStyleSheet(f"""
            QFrame {{
                background-color: white;
                border: 2px solid {color};
                border-radius: 12px;
            }}
        """)
        card.setFixedHeight(100)
        
        layout = QVBoxLayout(card)
        layout.setContentsMargins(15, 15, 15, 15)
        layout.setSpacing(8)
        
        title_label = QLabel(title)
        title_label.setStyleSheet(f"color: black; font-size: 13px; font-weight: bold; border: none;")
        
        amount_label = QLabel(amount)
        amount_label.setObjectName(f"amount_{title}")
        amount_label.setStyleSheet(f"color: black; font-size: 15px; font-weight: bold; border: none;")
        
        layout.addWidget(title_label)
        layout.addWidget(amount_label)
        layout.addStretch()
        
        return card
    
    def createTransactionSection(self):
        """Buat section untuk daftar transaksi"""
        section = QFrame()
        section.setStyleSheet("background-color: white; border: none;")
        
        layout = QVBoxLayout(section)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(15)
        
        # Scroll area untuk transaction list
        scroll = QScrollArea()
        scroll.setWidgetResizable(True)
        scroll.setStyleSheet("""
            QScrollArea { border: none; background-color: white; }
            QScrollBar:vertical { width: 10px; }
        """)
        
        self.transactions_container = QWidget()
        self.transactions_layout = QVBoxLayout(self.transactions_container)
        self.transactions_layout.setContentsMargins(0, 0, 0, 0)
        self.transactions_layout.setSpacing(0)
        
        # Load real transaction data from database
        self.load_transactions()
        self.updateTransactionsList()
        
        self.transactions_layout.addStretch()
        scroll.setWidget(self.transactions_container)
        layout.addWidget(scroll, 1)
        
        return section
    
    def updateTransactionsList(self):
        """Update tampilan daftar transaksi berdasarkan filter"""
        # Clear layout
        while self.transactions_layout.count():
            child = self.transactions_layout.takeAt(0)
            if child.widget():
                child.widget().deleteLater()
        
        # Filter transaksi
        filtered_transactions = self.filterTransactions()
        
        # Group by month/week/day
        grouped = self.groupTransactions(filtered_transactions)
        
        # Display grouped transactions
        for group_label, transactions in grouped.items():
            # Group header (clickable)
            header_frame = QFrame()
            header_frame.setCursor(Qt.CursorShape.PointingHandCursor)
            header_frame.setStyleSheet("""
                QFrame:hover {
                    background-color: #F0F2FF;
                    border-radius: 8px;
                }
            """)
            
            header_layout = QHBoxLayout(header_frame)
            header_layout.setContentsMargins(0, 10, 0, 10)
            
            header = QLabel(group_label)
            header.setStyleSheet("font-size: 16px; font-weight: bold; color: black; border: none;")
            header_layout.addWidget(header)
            
            # Make header clickable for report
            header_frame.mousePressEvent = lambda e, label=group_label: self.onGroupHeaderClicked(label)
            
            self.transactions_layout.addWidget(header_frame)
            
            # Transactions
            for trans in transactions:
                trans_card = self.createTransactionCard(trans)
                self.transactions_layout.addWidget(trans_card)
        
        self.transactions_layout.addStretch()
    
    def filterTransactions(self):
        """Filter transaksi berdasarkan current filter"""
        # PERBAIKAN: Untuk Harian, Mingguan, Bulanan -> Tampilkan SEMUA riwayat
        # Agar user bisa scroll ke bawah
        if self.current_filter in ["Harian", "Mingguan", "Bulanan"]:
            return self.transactions_data
        
        # Pilih Tanggal 
        elif self.current_filter == "Pilih Tanggal":
            if self.current_start_date and self.current_end_date:
                end_of_day = self.current_end_date.replace(hour=23, minute=59, second=59)
                return [t for t in self.transactions_data 
                        if self.current_start_date <= t["date"] <= end_of_day]
        
        return self.transactions_data
    
    def groupTransactions(self, transactions):
        """Group transaksi berdasarkan filter type"""
        grouped = defaultdict(list)
        
        # Sort transaksi dari yang terbaru
        sorted_transactions = sorted(transactions, key=lambda x: x["date"], reverse=True)

        if self.current_filter == "Harian":
            today = datetime.now().date()
            yesterday = today - timedelta(days=1)
            
            for trans in sorted_transactions:
                t_date = trans["date"].date()
                
                # Label khusus untuk Hari Ini dan Kemarin
                if t_date == today:
                    day_str = trans["date"].strftime("%A, %d %b %Y")
                    group_label = f"Hari ini ({day_str})"
                elif t_date == yesterday:
                    day_str = trans["date"].strftime("%A, %d %b %Y")
                    group_label = f"Kemarin ({day_str})"
                else:
                    # Format: "Saturday, 22 Nov 2025"
                    group_label = trans["date"].strftime("%A, %d %b %Y")
                
                grouped[group_label].append(trans)
        
        elif self.current_filter == "Mingguan":
            for trans in sorted_transactions:
                tx_date = trans["date"]
                start_of_week = tx_date - timedelta(days=tx_date.weekday())
                end_of_week = start_of_week + timedelta(days=6)
                
                s_str = start_of_week.strftime("%d")
                e_str = end_of_week.strftime("%d %B %Y")
                if start_of_week.month != end_of_week.month:
                    s_str = start_of_week.strftime("%d %b")
                
                group_label = f"{s_str} - {e_str}"
                grouped[group_label].append(trans)
        
        elif self.current_filter == "Bulanan":
            for trans in sorted_transactions:
                # Format: "November 2025", "October 2025", dst.
                month_label = trans["date"].strftime("%B %Y")
                grouped[month_label].append(trans)
        
        elif self.current_filter == "Pilih Tanggal":
            if self.current_start_date and self.current_end_date:
                start_str = self.current_start_date.strftime("%d")
                end_str = self.current_end_date.strftime("%d")
                if self.current_start_date.month == self.current_end_date.month:
                    month_year = self.current_start_date.strftime("%B %Y")
                    group_label = f"{start_str} - {end_str} {month_year}"
                else:
                    start_full = self.current_start_date.strftime("%d %B")
                    end_full = self.current_end_date.strftime("%d %B %Y")
                    group_label = f"{start_full} - {end_full}"
                
                for trans in sorted_transactions:
                    grouped[group_label].append(trans)
        
        return grouped
    
    def createTransactionCard(self, transaction):
        """Buat kartu transaksi"""
        card = QFrame()
        card.setStyleSheet("""
            QFrame {
                background-color: white;
                border: none;
                border-bottom: 1px solid #E0E0E0;
            }
        """)
        card.setCursor(Qt.CursorShape.PointingHandCursor)
        card.setFixedHeight(80)
        
        layout = QHBoxLayout(card)
        layout.setContentsMargins(0, 15, 0, 15)
        layout.setSpacing(15)
        
        # Icon
        icon_label = QLabel()
        icon_label.setFixedSize(32, 32)
        icon = QIcon("img/Rekening.svg")
        if not icon.isNull():
            icon_label.setPixmap(icon.pixmap(32, 32))
        
        # Description and wallet
        info_layout = QVBoxLayout()
        info_layout.setSpacing(4)
        
        desc_label = QLabel(transaction["description"])
        desc_label.setStyleSheet("font-size: 14px; font-weight: bold; color: black; border: none;")
        
        wallet_label = QLabel(transaction["wallet"])
        wallet_label.setStyleSheet("font-size: 12px; color: #808080; border: none;")
        
        info_layout.addWidget(desc_label)
        info_layout.addWidget(wallet_label)
        
        # Date and amount
        date_label = QLabel(transaction["date"].strftime("%d %b %Y"))
        date_label.setStyleSheet("font-size: 12px; color: #808080; border: none;")
        date_label.setAlignment(Qt.AlignmentFlag.AlignRight)
        
        amount_text = f"+Rp{transaction['amount']:,}" if transaction["type"] == "income" else f"-Rp{transaction['amount']:,}"
        amount_color = "#2ECC71" if transaction["type"] == "income" else "#E74C3C"
        amount_label = QLabel(amount_text)
        amount_label.setStyleSheet(f"font-size: 13px; font-weight: bold; color: {amount_color}; border: none;")
        amount_label.setAlignment(Qt.AlignmentFlag.AlignRight)
        
        right_layout = QVBoxLayout()
        right_layout.setSpacing(4)
        right_layout.addWidget(date_label)
        right_layout.addWidget(amount_label)
        
        layout.addWidget(icon_label)
        layout.addLayout(info_layout)
        layout.addStretch()
        layout.addLayout(right_layout)
        
        # Store transaction data in card for double-click access
        card.setProperty("transaction_data", transaction)
        
        # Click handler - single click
        card.mousePressEvent = lambda e: self.onTransactionClicked(transaction["id"])
        
        # Double-click handler - open edit form
        card.mouseDoubleClickEvent = lambda e: self.onTransactionDoubleClicked(transaction)
        
        return card
    
    def onTransactionDoubleClicked(self, transaction):
        """Handle double-click on transaction - open edit form"""
        self.transaction_edit_requested.emit(transaction["id"], transaction)
    
    def onFilterClicked(self, filter_name):
        """Handle klik filter button"""
        self.current_filter = filter_name
        
        if filter_name == "Pilih Tanggal":
            # Show date range dialog
            dialog = DateRangeDialog(self)
            if dialog.exec():
                start, end = dialog.getDateRange()
                self.current_start_date = start
                self.current_end_date = end
                print(f"Date range: {start} to {end}")
        
        self.updateFilterButtonsStyle()
        self.updateTransactionsList()
        self.updateSummaryCards()
    
    def updateSummaryCards(self):
        """Update summary cards berdasarkan filtered transactions"""
        filtered = self.filterTransactions()
        
        total_income = sum(t["amount"] for t in filtered if t["type"] == "income")
        total_expense = sum(t["amount"] for t in filtered if t["type"] == "expense")
        transaction_count = len(filtered)
        
        # Update labels (simplified)
        income_label = self.income_card.findChild(QLabel, "amount_Pendapatan")
        if income_label:
            income_label.setText(f"Rp{total_income:,.0f}")
        
        expense_label = self.expense_card.findChild(QLabel, "amount_Pengeluaran")
        if expense_label:
            expense_label.setText(f"Rp{total_expense:,.0f}")
        
        savings_label = self.savings_card.findChild(QLabel, "amount_Tabungan")
        if savings_label:
            savings_label.setText(f"Rp{total_income - total_expense:,.0f}")
        
        payment_label = self.payment_card.findChild(QLabel, "amount_Jumlah Transaksi")
        if payment_label:
            payment_label.setText(f"{transaction_count} transaksi")
    
    def updateFilterButtonsStyle(self):
        """Update styling untuk filter buttons"""
        filters = ["Harian", "Mingguan", "Bulanan", "Pilih Tanggal"]
        
        for filter_name in filters:
            btn = self.findChild(QPushButton, f"filter_{filter_name}")
            if btn:
                if filter_name == self.current_filter:
                    btn.setStyleSheet("""
                        QPushButton {
                            background-color: #5873E8;
                            color: white;
                            border-radius: 12px;
                            font-weight: bold;
                            font-size: 13px;
                        }
                        QPushButton:hover { background-color: #4A63D0; }
                    """)
                else:
                    btn.setStyleSheet("""
                        QPushButton {
                            background-color: white;
                            color: #5873E8;
                            border: 2px solid #5873E8;
                            border-radius: 12px;
                            font-weight: bold;
                            font-size: 13px;
                        }
                        QPushButton:hover { background-color: #F0F2FF; }
                    """)
    
    def onTransactionClicked(self, transaction_id):
        """Handle klik transaksi"""
        self.transaction_clicked.emit(transaction_id)
        print(f"Transaction clicked: {transaction_id}")
    
    def onGroupHeaderClicked(self, group_label):
        """Handle klik group header untuk generate laporan"""
        start_date = None
        end_date = None

        if self.current_filter == "Harian":
            today = datetime.now()
            start_date = today.replace(hour=0, minute=0, second=0)
            end_date = today.replace(hour=23, minute=59, second=59)
        
        elif self.current_filter == "Mingguan":
            # PERBAIKAN: Parse dari format "17 - 23 November 2025"
            try:
                parts = group_label.split(" - ")
                date_end_str = parts[-1] # "23 November 2025"
                
                end_date_obj = datetime.strptime(date_end_str, "%d %B %Y")
                
                # Set End Date (Akhir hari Minggu)
                end_date = end_date_obj.replace(hour=23, minute=59, second=59)
                
                # Set Start Date (Mundur 6 hari ke Senin)
                start_date_obj = end_date_obj - timedelta(days=6)
                start_date = start_date_obj.replace(hour=0, minute=0, second=0)
            except Exception as e:
                print(f"Error parsing weekly date: {e}")
                return
        
        elif self.current_filter == "Bulanan":
            try:
                month_obj = datetime.strptime(group_label, "%B %Y")
                start_date = month_obj.replace(day=1, hour=0, minute=0, second=0)
                # Last day of month logic
                if month_obj.month == 12:
                    end_date = month_obj.replace(day=31, hour=23, minute=59, second=59)
                else:
                    next_month = month_obj.replace(month=month_obj.month + 1, day=1)
                    end_date = (next_month - timedelta(days=1)).replace(hour=23, minute=59, second=59)
            except:
                return
        
        elif self.current_filter == "Pilih Tanggal":
            if self.current_start_date and self.current_end_date:
                start_date = self.current_start_date.replace(hour=0, minute=0, second=0)
                end_date = self.current_end_date.replace(hour=23, minute=59, second=59)
            else:
                return

    def load_transactions(self):
        """Load real transactions from TransactionController"""
        if not self.transaction_controller or not self.account_controller:
            return
        
        accounts = self.account_controller.get_user_accounts(self.user.user_id)
        self.transactions_data = []
        
        for account in accounts:
            transactions = self.transaction_controller.get_account_transactions(account.account_id)
            for trans in transactions:
                self.transactions_data.append({
                    "id": trans.transaction_id,
                    "account_id": trans.account_id,
                    "wallet": account.account_name,  # Add account name for display
                    "amount": trans.amount,
                    "type": trans.transaction_type.value,
                    "description": trans.description,
                    "date": trans.date
                })
        
        # Sort by date descending
        self.transactions_data.sort(key=lambda x: x["date"], reverse=True)

    def _get_transaction_summary(self) -> dict:
        """Get transaction summary for current user"""
        if not self.transaction_controller or not self.account_controller:
            return {"income": 0, "expense": 0, "savings": 0, "count": 0}
        
        accounts = self.account_controller.get_user_accounts(self.user.user_id)
        total_income = 0.0
        total_expense = 0.0
        count = 0
        
        for account in accounts:
            transactions = self.transaction_controller.get_account_transactions(account.account_id)
            for trans in transactions:
                if trans.is_income():
                    total_income += trans.amount
                else:
                    total_expense += trans.amount
                count += 1
        
        return {
            "income": total_income,
            "expense": total_expense,
            "savings": total_income - total_expense,
            "count": count
        }

