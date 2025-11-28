import sys
from PyQt6.QtWidgets import (
    QApplication, QWidget, QLabel, QPushButton, QVBoxLayout, QHBoxLayout, 
    QFrame, QScrollArea, QSizePolicy, QProgressBar, QGraphicsDropShadowEffect,
    QMessageBox, QButtonGroup, QDialog
)
from PyQt6.QtCore import Qt, pyqtSignal, QSize
from PyQt6.QtGui import QIcon, QFont, QColor
import matplotlib
matplotlib.use('QtAgg')
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.figure import Figure
import numpy as np
from datetime import datetime, timedelta
from collections import defaultdict

# Menggunakan QFont.Weight.Bold untuk kompatibilitas PyQt6
BOLD = QFont.Weight.Bold 


class KonfirmasiHapusTargetDialog(QDialog):
    """Custom dialog untuk konfirmasi hapus target"""
    
    def __init__(self, parent, target_name):
        super().__init__(parent)
        self.target_name = target_name
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
            f"Apakah Anda yakin ingin menghapus target '{self.target_name}'?"
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


class ProgressChart(FigureCanvas):
    """Dynamic chart that displays transaction data based on time interval."""
    
    def __init__(self, transaction_controller=None, account_controller=None, user=None, selected_account_id=None):
        self.fig = Figure(figsize=(5, 4), dpi=100)
        self.fig.patch.set_facecolor('none') 
        super().__init__(self.fig)
        
        self.transaction_controller = transaction_controller
        self.account_controller = account_controller
        self.user = user
        self.selected_account_id = selected_account_id  # Filter by specific account
        self.current_interval = "Mingguan"  # Default interval
        
        self.setMinimumHeight(300)
        self.setStyleSheet("background-color: transparent;")
        
        self.ax = self.fig.add_subplot(111)
        self.ax.set_facecolor('none')
        
        # Initial render with default interval
        self.update_chart("Mingguan")
    
    def update_chart(self, interval: str):
        """Update chart with real transaction data based on interval."""
        self.current_interval = interval
        self.ax.clear()
        self.ax.set_facecolor('none')
        
        # Get transaction data
        data = self._get_transaction_data(interval)
        x_labels = data["labels"]
        income_values = data["income"]
        expense_values = data["expense"]
        
        if not x_labels:
            # No data - show empty state
            self.ax.text(0.5, 0.5, "Belum ada data transaksi", 
                        ha='center', va='center', transform=self.ax.transAxes,
                        fontsize=12, color='#808080')
            self.draw()
            return
        
        x = np.arange(len(x_labels))
        width = 0.35  # Width of bars
        
        # Create bar chart for income and expense
        bars_income = self.ax.bar(x - width/2, income_values, width, 
                                   label='Pemasukan', color='#2ECC71', alpha=0.8)
        bars_expense = self.ax.bar(x + width/2, expense_values, width, 
                                    label='Pengeluaran', color='#E74C3C', alpha=0.8)
        
        # Also plot net savings as a line
        net_values = [inc - exp for inc, exp in zip(income_values, expense_values)]
        line, = self.ax.plot(x, net_values, color='#5873E8', linewidth=2, 
                             marker='o', markerfacecolor='white', 
                             markeredgecolor='#5873E8', markersize=6,
                             label='Tabungan Bersih', zorder=5)
        
        # Fill area under net savings line
        self.ax.fill_between(x, net_values, 0, 
                             where=[n >= 0 for n in net_values],
                             color='#5873E8', alpha=0.1, interpolate=True)
        self.ax.fill_between(x, net_values, 0,
                             where=[n < 0 for n in net_values],
                             color='#E74C3C', alpha=0.1, interpolate=True)
        
        # Styling Grid
        self.ax.grid(True, axis='y', linestyle='--', alpha=0.5, color='#C0C0C0')
        self.ax.set_axisbelow(True)
        
        # Hide Spines
        self.ax.spines['top'].set_visible(False)
        self.ax.spines['right'].set_visible(False)
        self.ax.spines['left'].set_visible(False)
        self.ax.spines['bottom'].set_visible(False)
        
        # X-axis
        self.ax.set_xticks(x)
        self.ax.set_xticklabels(x_labels, color='#808080', fontsize=8, rotation=45, ha='right')
        self.ax.tick_params(axis='x', length=0)
        
        # Y-axis - dynamic based on data
        max_val = max(max(income_values) if income_values else 0, 
                      max(expense_values) if expense_values else 0,
                      max(net_values) if net_values else 0)
        min_val = min(min(net_values) if net_values else 0, 0)
        
        # Format Y-axis labels in thousands/millions
        def format_currency(val):
            if abs(val) >= 1_000_000:
                return f"{val/1_000_000:.1f}jt"
            elif abs(val) >= 1_000:
                return f"{val/1_000:.0f}rb"
            else:
                return f"{val:.0f}"
        
        # Set Y limits with padding
        y_padding = (max_val - min_val) * 0.1 if max_val != min_val else 1000
        self.ax.set_ylim(min_val - y_padding, max_val + y_padding)
        
        # Format Y tick labels
        yticks = self.ax.get_yticks()
        self.ax.set_yticks(yticks)
        self.ax.set_yticklabels([format_currency(y) for y in yticks], color='#808080', fontsize=8)
        self.ax.tick_params(axis='y', length=0)
        
        # Add legend
        self.ax.legend(loc='upper left', fontsize=8, framealpha=0.9)
        
        self.fig.tight_layout()
        self.draw()
    
    def set_selected_account(self, account_id):
        """Set the selected account to filter transactions."""
        self.selected_account_id = account_id
        self.update_chart(self.current_interval)
    
    def _get_transaction_data(self, interval: str) -> dict:
        """Fetch and aggregate transaction data based on interval."""
        if not self.transaction_controller or not self.account_controller or not self.user:
            return {"labels": [], "income": [], "expense": []}
        
        # Get accounts based on selection
        if self.selected_account_id:
            # Filter by specific account
            all_accounts = self.account_controller.get_user_accounts(self.user.user_id)
            accounts = [acc for acc in all_accounts if acc.account_id == self.selected_account_id]
        else:
            # Get all user accounts
            accounts = self.account_controller.get_user_accounts(self.user.user_id)
        
        if not accounts:
            return {"labels": [], "income": [], "expense": []}
        
        # Collect transactions from selected account(s)
        all_transactions = []
        for account in accounts:
            transactions = self.transaction_controller.get_account_transactions(account.account_id)
            for trans in transactions:
                all_transactions.append({
                    "date": trans.date,
                    "amount": trans.amount,
                    "type": "income" if trans.is_income() else "expense"
                })
        
        if not all_transactions:
            return {"labels": [], "income": [], "expense": []}
        
        # Aggregate based on interval
        if interval == "Harian":
            return self._aggregate_daily(all_transactions)
        elif interval == "Mingguan":
            return self._aggregate_weekly(all_transactions)
        elif interval == "Bulanan":
            return self._aggregate_monthly(all_transactions)
        
        return {"labels": [], "income": [], "expense": []}
    
    def _aggregate_daily(self, transactions: list) -> dict:
        """Aggregate transactions by day (last 7 days)."""
        today = datetime.now().date()
        
        # Initialize last 7 days
        days = []
        for i in range(6, -1, -1):
            day = today - timedelta(days=i)
            days.append(day)
        
        # Aggregate
        income_by_day = defaultdict(float)
        expense_by_day = defaultdict(float)
        
        for trans in transactions:
            trans_date = trans["date"].date()
            if trans_date in days:
                if trans["type"] == "income":
                    income_by_day[trans_date] += trans["amount"]
                else:
                    expense_by_day[trans_date] += trans["amount"]
        
        # Build result
        labels = []
        income_values = []
        expense_values = []
        
        day_names = ["Sen", "Sel", "Rab", "Kam", "Jum", "Sab", "Min"]
        
        for day in days:
            labels.append(f"{day_names[day.weekday()]}\n{day.day}")
            income_values.append(income_by_day.get(day, 0))
            expense_values.append(expense_by_day.get(day, 0))
        
        return {"labels": labels, "income": income_values, "expense": expense_values}
    
    def _aggregate_weekly(self, transactions: list) -> dict:
        """Aggregate transactions by week (last 4 weeks)."""
        today = datetime.now().date()
        
        # Find start of current week (Monday)
        start_of_week = today - timedelta(days=today.weekday())
        
        # Get last 4 weeks
        weeks = []
        for i in range(3, -1, -1):
            week_start = start_of_week - timedelta(weeks=i)
            week_end = week_start + timedelta(days=6)
            weeks.append((week_start, week_end))
        
        # Aggregate
        income_by_week = defaultdict(float)
        expense_by_week = defaultdict(float)
        
        for trans in transactions:
            trans_date = trans["date"].date()
            for week_start, week_end in weeks:
                if week_start <= trans_date <= week_end:
                    key = week_start
                    if trans["type"] == "income":
                        income_by_week[key] += trans["amount"]
                    else:
                        expense_by_week[key] += trans["amount"]
                    break
        
        # Build result
        labels = []
        income_values = []
        expense_values = []
        
        for week_start, week_end in weeks:
            labels.append(f"{week_start.day}-{week_end.day}\n{week_start.strftime('%b')}")
            income_values.append(income_by_week.get(week_start, 0))
            expense_values.append(expense_by_week.get(week_start, 0))
        
        return {"labels": labels, "income": income_values, "expense": expense_values}
    
    def _aggregate_monthly(self, transactions: list) -> dict:
        """Aggregate transactions by month (last 6 months)."""
        today = datetime.now().date()
        
        # Get last 6 months
        months = []
        for i in range(5, -1, -1):
            month_date = today - timedelta(days=30 * i)
            months.append((month_date.year, month_date.month))
        
        # Remove duplicates and sort
        months = list(dict.fromkeys(months))
        months = sorted(months)[-6:]  # Last 6 unique months
        
        # Aggregate
        income_by_month = defaultdict(float)
        expense_by_month = defaultdict(float)
        
        for trans in transactions:
            trans_date = trans["date"]
            key = (trans_date.year, trans_date.month)
            if key in months:
                if trans["type"] == "income":
                    income_by_month[key] += trans["amount"]
                else:
                    expense_by_month[key] += trans["amount"]
        
        # Build result
        labels = []
        income_values = []
        expense_values = []
        
        month_names = ["", "Jan", "Feb", "Mar", "Apr", "Mei", "Jun", 
                       "Jul", "Agu", "Sep", "Okt", "Nov", "Des"]
        
        for year, month in months:
            labels.append(f"{month_names[month]}\n{year}")
            income_values.append(income_by_month.get((year, month), 0))
            expense_values.append(expense_by_month.get((year, month), 0))
        
        return {"labels": labels, "income": income_values, "expense": expense_values}

class ProgressView(QWidget):
    # Signal emitted when user wants to add a new target
    add_target_requested = pyqtSignal()
    # Signal emitted when user double-clicks a target to edit (target_id, target_data)
    target_edit_requested = pyqtSignal(str, dict)
    # Signal emitted when target is successfully deleted
    target_deleted = pyqtSignal()
    
    def __init__(self, user=None, account_controller=None, target_controller=None, transaction_controller=None):
        super().__init__()
        self.user = user
        self.account_controller = account_controller
        self.target_controller = target_controller
        self.transaction_controller = transaction_controller
        self.current_interval = "Mingguan"  # Default interval
        self.chart = None  # Will hold reference to chart widget
        self.interval_buttons = {}  # Will hold references to interval buttons
        self.account_buttons = {}  # Will hold references to account buttons
        self.selected_account_id = None  # Currently selected account (None = all accounts)
        self.setStyleSheet("background-color: white;")
        
        # Main Layout
        main_layout = QHBoxLayout()
        main_layout.setContentsMargins(0, 0, 0, 0)
        main_layout.setSpacing(0)
        self.setLayout(main_layout)
        
        # Scroll Area Content (Vertikal)
        scroll_area = QScrollArea()
        scroll_area.setWidgetResizable(True)
        scroll_area.setStyleSheet("""
            QScrollArea { border: none; background-color: white; }
            QScrollBar:vertical { width: 10px; }
        """)
        
        self.content_widget = QWidget()
        self.content_layout = QVBoxLayout(self.content_widget)
        self.content_layout.setContentsMargins(40, 40, 40, 40)
        self.content_layout.setSpacing(30)
        
        # --- Bagian Atas: Progress Menabung & Cards (Horizontal Scrollable) ---
        self.setup_top_section()

        # --- Bagian Tengah: Chart with Interval Selector ---
        self.setup_chart_section()

        # --- Bagian Bawah: Target Tabungan ---
        self.setup_target_section()
        
        self.content_layout.addStretch()
        scroll_area.setWidget(self.content_widget)
        main_layout.addWidget(scroll_area)
    
    def refresh(self):
        """Refresh view with updated data from database."""
        # Clear existing content layout - delete widgets properly
        self._clear_layout(self.content_layout)
        
        # Rebuild UI sections
        self.setup_top_section()
        self.setup_chart_section()
        self.setup_target_section()
        self.content_layout.addStretch()
    
    def _clear_layout(self, layout):
        """Recursively clear a layout and its children."""
        if layout is None:
            return
        while layout.count():
            child = layout.takeAt(0)
            if child.widget():
                widget = child.widget()
                widget.setParent(None)  # Remove from parent immediately
                widget.deleteLater()
            elif child.layout():
                self._clear_layout(child.layout())


    def setup_top_section(self):
        """Judul Halaman dan Kartu Wallet Horizontal Scrollable"""
        title = QLabel("Daftar dan Progress Target")
        title.setStyleSheet("font-size: 24px; font-weight: bold; color: black;")
        self.content_layout.addWidget(title)

        # 1. Kontainer untuk Horizontal Scrolling
        scroll_container = QScrollArea()
        scroll_container.setWidgetResizable(True)
        
        # --- PERBAIKAN: Tinggi tetap (Fix agar kartu Gopay/Dana muncul dan tidak terpotong) ---
        scroll_container.setFixedHeight(120) 
        # ---------------------------------------------------------------------------------------------------------------------
        
        scroll_container.setHorizontalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAsNeeded)
        scroll_container.setVerticalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAlwaysOff)
        scroll_container.setStyleSheet("""
            QScrollArea { border: none; }
            QScrollBar:horizontal {
                border: none;
                background: #f0f0f0;
                height: 8px;
                margin: 0px 0px 0px 0px;
                border-radius: 4px;
            }
            QScrollBar::handle:horizontal {
                background: #C0C0C0;
                min-width: 20px;
                border-radius: 4px;
            }
            QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
                border: none;
                background: none;
            }
        """)

        # 2. Widget Konten di dalam Scroll Area
        scroll_content_widget = QWidget()
        cards_layout = QHBoxLayout(scroll_content_widget)
        cards_layout.setContentsMargins(0, 0, 0, 0)
        cards_layout.setSpacing(20)
        
        # Reset account buttons dictionary
        self.account_buttons = {}
        
        # Add "Semua Rekening" button first
        all_accounts_btn = self.create_wallet_button("Semua Rekening", 0, 0, account_id=None)
        cards_layout.addWidget(all_accounts_btn)
        self.account_buttons[None] = all_accounts_btn
        
        # Load real accounts from database
        if self.user and self.account_controller:
            accounts = self.account_controller.get_user_accounts(self.user.user_id)
            for account in accounts:
                btn = self.create_wallet_button(
                    account.account_name, 
                    account.balance, 
                    account.balance,
                    account_id=account.account_id
                )
                cards_layout.addWidget(btn)
                self.account_buttons[account.account_id] = btn
        
        # Tetapkan scroll_content_widget sebagai widget dari QScrollArea
        scroll_container.setWidget(scroll_content_widget)
        
        self.content_layout.addWidget(scroll_container)

    def setup_chart_section(self):
        """Grafik Chart dengan Interval Selector"""
        # Container for chart section
        chart_section = QFrame()
        chart_section.setStyleSheet("""
            QFrame {
                background-color: #F8F9FE;
                border-radius: 15px;
            }
        """)
        
        section_layout = QVBoxLayout(chart_section)
        section_layout.setContentsMargins(20, 20, 20, 20)
        section_layout.setSpacing(15)
        
        # Header with title and interval selector
        header_layout = QHBoxLayout()
        
        chart_title = QLabel("Statistik Transaksi")
        chart_title.setStyleSheet("font-size: 18px; font-weight: bold; color: black; background: transparent;")
        header_layout.addWidget(chart_title)
        
        header_layout.addStretch()
        
        # Interval selector buttons
        intervals = ["Harian", "Mingguan", "Bulanan"]
        self.interval_button_group = QButtonGroup(self)
        self.interval_button_group.setExclusive(True)
        self.interval_buttons = {}
        
        for interval in intervals:
            btn = QPushButton(interval)
            btn.setCheckable(True)
            btn.setFixedSize(90, 32)
            btn.setCursor(Qt.CursorShape.PointingHandCursor)
            
            if interval == self.current_interval:
                btn.setChecked(True)
                btn.setStyleSheet("""
                    QPushButton {
                        background-color: #5873E8;
                        color: white;
                        border-radius: 16px;
                        font-weight: bold;
                        font-size: 12px;
                    }
                """)
            else:
                btn.setStyleSheet("""
                    QPushButton {
                        background-color: white;
                        color: #5873E8;
                        border: 1px solid #5873E8;
                        border-radius: 16px;
                        font-weight: bold;
                        font-size: 12px;
                    }
                    QPushButton:hover { background-color: #F0F2FF; }
                """)
            
            btn.clicked.connect(lambda checked, i=interval: self._on_interval_changed(i))
            self.interval_button_group.addButton(btn)
            self.interval_buttons[interval] = btn
            header_layout.addWidget(btn)
        
        section_layout.addLayout(header_layout)
        
        # Chart widget - now with real data, filtered by selected account
        self.chart = ProgressChart(
            transaction_controller=self.transaction_controller,
            account_controller=self.account_controller,
            user=self.user,
            selected_account_id=self.selected_account_id
        )
        section_layout.addWidget(self.chart)
        
        # Legend explanation
        legend_layout = QHBoxLayout()
        legend_layout.setSpacing(20)
        legend_layout.addStretch()
        
        # Income legend
        income_dot = QFrame()
        income_dot.setFixedSize(12, 12)
        income_dot.setStyleSheet("background-color: #2ECC71; border-radius: 6px;")
        income_label = QLabel("Pemasukan")
        income_label.setStyleSheet("color: #606060; font-size: 11px; background: transparent;")
        legend_layout.addWidget(income_dot)
        legend_layout.addWidget(income_label)
        
        # Expense legend
        expense_dot = QFrame()
        expense_dot.setFixedSize(12, 12)
        expense_dot.setStyleSheet("background-color: #E74C3C; border-radius: 6px;")
        expense_label = QLabel("Pengeluaran")
        expense_label.setStyleSheet("color: #606060; font-size: 11px; background: transparent;")
        legend_layout.addWidget(expense_dot)
        legend_layout.addWidget(expense_label)
        
        # Net savings legend
        savings_dot = QFrame()
        savings_dot.setFixedSize(12, 12)
        savings_dot.setStyleSheet("background-color: #5873E8; border-radius: 6px;")
        savings_label = QLabel("Tabungan Bersih")
        savings_label.setStyleSheet("color: #606060; font-size: 11px; background: transparent;")
        legend_layout.addWidget(savings_dot)
        legend_layout.addWidget(savings_label)
        
        legend_layout.addStretch()
        section_layout.addLayout(legend_layout)
        
        self.content_layout.addWidget(chart_section)
    
    def _on_interval_changed(self, interval: str):
        """Handle interval button click - update chart."""
        self.current_interval = interval
        
        # Update button styles
        for btn_interval, btn in self.interval_buttons.items():
            if btn_interval == interval:
                btn.setStyleSheet("""
                    QPushButton {
                        background-color: #5873E8;
                        color: white;
                        border-radius: 16px;
                        font-weight: bold;
                        font-size: 12px;
                    }
                """)
            else:
                btn.setStyleSheet("""
                    QPushButton {
                        background-color: white;
                        color: #5873E8;
                        border: 1px solid #5873E8;
                        border-radius: 16px;
                        font-weight: bold;
                        font-size: 12px;
                    }
                    QPushButton:hover { background-color: #F0F2FF; }
                """)
        
        # Update chart
        if self.chart:
            self.chart.update_chart(interval)

    def setup_target_section(self):
        """Header Target dan List Kartu Target"""
        
        # Header Row: Judul kiri, Button kanan
        header_layout = QHBoxLayout()
        lbl_title = QLabel("Target Tabungan")
        lbl_title.setStyleSheet("font-size: 20px; font-weight: bold; color: black;")
        
        btn_add = QPushButton("Tambah target  >")
        btn_add.setFixedSize(160, 40)
        btn_add.setCursor(Qt.CursorShape.PointingHandCursor)
        btn_add.setStyleSheet("""
            QPushButton {
                background-color: #5873E8;
                color: white;
                font-weight: bold;
                border-radius: 20px;
                font-size: 14px;
            }
            QPushButton:hover { background-color: #4A63D0; }
        """)
        # Connect button to signal for navigation
        btn_add.clicked.connect(self.add_target_requested.emit)
        
        header_layout.addWidget(lbl_title)
        header_layout.addStretch()
        header_layout.addWidget(btn_add)
        
        self.content_layout.addLayout(header_layout)

        # List Target Cards - Load real targets from ALL user accounts
        targets_layout = QVBoxLayout()
        targets_layout.setSpacing(15)

        # Load real targets for all user accounts
        targets = []
        if self.user and self.account_controller and self.target_controller:
            accounts = self.account_controller.get_user_accounts(self.user.user_id)
            for account in accounts:
                target_objects = self.target_controller.get_account_targets(account.account_id)
                for target_obj in target_objects:
                    if not target_obj.is_archived:
                        # Use target's current_amount for progress tracking
                        targets.append({
                            "title": target_obj.target_name,
                            "wallet": account.account_name,
                            "curr": target_obj.current_amount,  # Progress is target's accumulated amount
                            "target": target_obj.target_amount,
                            "target_id": target_obj.target_id,
                            "account_id": account.account_id,
                            "deadline": target_obj.deadline
                        })
        
        if not targets:
            # Show empty state message
            empty_label = QLabel("Belum ada target tabungan. Klik 'Tambah target' untuk membuat target baru.")
            empty_label.setStyleSheet("color: #808080; font-style: italic; font-size: 14px;")
            empty_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
            targets_layout.addWidget(empty_label)
        else:
            for t in targets:
                card = self.create_target_card(
                    t["title"], t["wallet"], t["curr"], t["target"],
                    t["target_id"], t["account_id"], t.get("deadline")
                )
                targets_layout.addWidget(card)

        self.content_layout.addLayout(targets_layout)

    # --- Helper Methods untuk Membuat Kartu ---

    def create_wallet_button(self, name, current, target, account_id=None):
        """Clickable button for selecting which account's progress to display"""
        btn = QPushButton()
        btn.setFixedSize(250, 100)
        btn.setCursor(Qt.CursorShape.PointingHandCursor)
        
        # Check if this account is selected
        is_selected = (account_id == self.selected_account_id)
        
        # Style based on selection state
        if is_selected:
            btn.setStyleSheet("""
                QPushButton {
                    background-color: #5873E8;
                    border: 2px solid #4A63D0;
                    border-radius: 15px;
                    text-align: left;
                }
                QPushButton:hover {
                    background-color: #4A63D0;
                }
            """)
            text_color = "white"
            value_color = "#E0E4FF"
            progress_bg = "rgba(255, 255, 255, 0.3)"
            progress_chunk = "white"
        else:
            btn.setStyleSheet("""
                QPushButton {
                    background-color: white;
                    border: 1px solid #E0E4F5;
                    border-radius: 15px;
                    text-align: left;
                }
                QPushButton:hover {
                    background-color: #F8F9FE;
                    border: 2px solid #5873E8;
                }
            """)
            text_color = "black"
            value_color = "#5873E8"
            progress_bg = "#F0F2FF"
            progress_chunk = "#5873E8"
        
        # DropShadow
        shadow = QGraphicsDropShadowEffect()
        shadow.setBlurRadius(8)
        shadow.setOffset(0, 3)
        shadow.setColor(QColor(0, 0, 0, 60))
        btn.setGraphicsEffect(shadow)
        
        # Create a widget to hold the content inside the button
        content_widget = QWidget(btn)
        content_widget.setGeometry(0, 0, 250, 100)
        content_widget.setAttribute(Qt.WidgetAttribute.WA_TransparentForMouseEvents)
        content_widget.setStyleSheet("background: transparent;")
        
        layout = QVBoxLayout(content_widget)
        layout.setContentsMargins(20, 15, 20, 15)
        layout.setSpacing(8)
        
        # Row 1: Icon + Name
        top_row = QHBoxLayout()
        
        icon_lbl = QLabel()
        icon_lbl.setFixedSize(24, 24)
        icon_lbl.setAttribute(Qt.WidgetAttribute.WA_TransparentForMouseEvents)
        icon = QIcon("img/Rekening.svg")
        icon_lbl.setPixmap(icon.pixmap(QSize(24, 24)))
        
        name_lbl = QLabel(name)
        name_lbl.setAttribute(Qt.WidgetAttribute.WA_TransparentForMouseEvents)
        name_lbl.setStyleSheet(f"font-weight: bold; font-size: 14px; color: {text_color}; border: none; background: transparent;")
        
        top_row.addWidget(icon_lbl)
        top_row.addWidget(name_lbl)
        top_row.addStretch()
        
        layout.addLayout(top_row)
        
        # For "Semua Rekening" button, show different content
        if account_id is None:
            # Just show label for "all accounts"
            all_lbl = QLabel("Tampilkan semua rekening")
            all_lbl.setAttribute(Qt.WidgetAttribute.WA_TransparentForMouseEvents)
            all_lbl.setStyleSheet(f"color: {value_color}; font-size: 12px; border: none; background: transparent;")
            layout.addWidget(all_lbl)
            layout.addStretch()
        else:
            # Progress Bar
            pbar = QProgressBar()
            pbar.setAttribute(Qt.WidgetAttribute.WA_TransparentForMouseEvents)
            percent = int((current / target) * 100) if target > 0 else 0
            pbar.setValue(min(percent, 100))
            pbar.setFixedHeight(6)
            pbar.setTextVisible(False)
            pbar.setStyleSheet(f"""
                QProgressBar {{
                    background-color: {progress_bg};
                    border-radius: 3px;
                    border: none;
                }}
                QProgressBar::chunk {{
                    background-color: {progress_chunk};
                    border-radius: 3px;
                }}
            """)
            
            # Value Text
            val_text = f"Rp{current:,.0f}".replace(",", ".")
            val_lbl = QLabel(val_text)
            val_lbl.setAttribute(Qt.WidgetAttribute.WA_TransparentForMouseEvents)
            val_lbl.setStyleSheet(f"color: {value_color}; font-size: 12px; font-weight: bold; border: none; background: transparent;")
            
            layout.addWidget(pbar)
            layout.addWidget(val_lbl)
        
        # Connect click to selection handler
        btn.clicked.connect(lambda checked, aid=account_id: self._on_account_selected(aid))
        
        return btn
    
    def _on_account_selected(self, account_id):
        """Handle account button click - update chart to show selected account's data."""
        self.selected_account_id = account_id
        
        # Update button styles and inner label colors to reflect new selection
        for aid, btn in self.account_buttons.items():
            is_selected = (aid == account_id)
            
            if is_selected:
                btn.setStyleSheet("""
                    QPushButton {
                        background-color: #5873E8;
                        border: 2px solid #4A63D0;
                        border-radius: 15px;
                        text-align: left;
                    }
                    QPushButton:hover {
                        background-color: #4A63D0;
                    }
                """)
                text_color = "white"
                value_color = "#E0E4FF"
                progress_bg = "rgba(255, 255, 255, 0.3)"
                progress_chunk = "white"
            else:
                btn.setStyleSheet("""
                    QPushButton {
                        background-color: white;
                        border: 1px solid #E0E4F5;
                        border-radius: 15px;
                        text-align: left;
                    }
                    QPushButton:hover {
                        background-color: #F8F9FE;
                        border: 2px solid #5873E8;
                    }
                """)
                text_color = "black"
                value_color = "#5873E8"
                progress_bg = "#F0F2FF"
                progress_chunk = "#5873E8"
            
            # Update inner label colors
            content_widget = btn.findChild(QWidget)
            if content_widget:
                # Update name label
                for label in content_widget.findChildren(QLabel):
                    current_style = label.styleSheet()
                    if "font-weight: bold" in current_style and "font-size: 14px" in current_style:
                        # This is the name label
                        label.setStyleSheet(f"font-weight: bold; font-size: 14px; color: {text_color}; border: none; background: transparent;")
                    elif "font-size: 12px" in current_style:
                        # This is the value label or "Tampilkan semua" label
                        if "font-weight: bold" in current_style:
                            label.setStyleSheet(f"color: {value_color}; font-size: 12px; font-weight: bold; border: none; background: transparent;")
                        else:
                            label.setStyleSheet(f"color: {value_color}; font-size: 12px; border: none; background: transparent;")
                
                # Update progress bar
                for pbar in content_widget.findChildren(QProgressBar):
                    pbar.setStyleSheet(f"""
                        QProgressBar {{
                            background-color: {progress_bg};
                            border-radius: 3px;
                            border: none;
                        }}
                        QProgressBar::chunk {{
                            background-color: {progress_chunk};
                            border-radius: 3px;
                        }}
                    """)
        
        # Update chart to show selected account's data
        if self.chart:
            self.chart.set_selected_account(account_id)
    
    def create_wallet_card(self, name, current, target):
        """Kartu di bagian atas (Kotak dengan icon wallet) - DEPRECATED, use create_wallet_button"""
        card = QFrame()
        # Menggunakan ukuran Fixed agar bisa di-scroll
        card.setFixedSize(250, 100) 
        
        # DropShadow
        shadow = QGraphicsDropShadowEffect()
        shadow.setBlurRadius(8)      
        shadow.setOffset(0, 3)          
        shadow.setColor(QColor(0, 0, 0, 60)) 
        card.setGraphicsEffect(shadow)
        
        # Style
        card.setStyleSheet("""
            QFrame {
                background-color: white;
                border: none; 
                border-radius: 15px;
            }
        """)
        
        layout = QVBoxLayout(card)
        layout.setContentsMargins(20, 15, 20, 15)
        layout.setSpacing(10)
        
        # Baris 1: Icon + Nama
        top_row = QHBoxLayout()
        
        icon_lbl = QLabel()
        icon_lbl.setFixedSize(24, 24)
        
        icon = QIcon("img/Rekening.svg") 
        icon_lbl.setPixmap(icon.pixmap(QSize(24, 24)))
        
        name_lbl = QLabel(name)
        name_lbl.setStyleSheet("font-weight: bold; font-size: 14px; color: black; border: none;")
        
        top_row.addWidget(icon_lbl)
        top_row.addWidget(name_lbl)
        top_row.addStretch()
        
        # Baris 2: Progress Bar
        pbar = QProgressBar()
        # Menghindari ZeroDivisionError
        percent = int((current / target) * 100) if target > 0 else 0
        pbar.setValue(percent)
        pbar.setFixedHeight(6)
        pbar.setTextVisible(False)
        pbar.setStyleSheet("""
            QProgressBar {
                background-color: #F0F2FF;
                border-radius: 3px;
                border: none;
            }
            QProgressBar::chunk {
                background-color: #5873E8;
                border-radius: 3px;
            }
        """)
        
        # Baris 3: Text Value
        val_text = f"Rp{current:,.0f}".replace(",", ".") + f" / Rp{target:,.0f}".replace(",", ".")
        val_lbl = QLabel(val_text)
        val_lbl.setStyleSheet("color: #5873E8; font-size: 12px; font-weight: bold; border: none;")
        
        layout.addLayout(top_row)
        layout.addWidget(pbar)
        layout.addWidget(val_lbl)
        
        return card

    def create_target_card(self, title, wallet_name, current, target, target_id=None, account_id=None, deadline=None):
        """Kartu Target di bagian bawah"""
        container = QWidget() 
        wrapper_layout = QVBoxLayout(container)
        wrapper_layout.setContentsMargins(0, 0, 0, 0)
        wrapper_layout.setSpacing(5)

        # Cap current at target if exceeded (for display purposes)
        display_current = min(current, target)

        # Kartu Utama
        card = QFrame()
        card.setFixedHeight(100)
        card.setStyleSheet("""
            QFrame {
                background-color: white;
                border: 1px solid #A0A0A0;
                border-radius: 15px;
            }
            QFrame:hover {
                background-color: #F8F9FE;
                border: 1px solid #5873E8;
            }
        """)
        card.setCursor(Qt.CursorShape.PointingHandCursor)
        card.setToolTip("Double-click untuk edit target")
        
        # Add double-click handler for editing
        def mouseDoubleClickEvent(a0):
            self._on_target_double_clicked(target_id, title, target, account_id, deadline)
        card.mouseDoubleClickEvent = mouseDoubleClickEvent
        
        card_layout = QVBoxLayout(card)
        card_layout.setContentsMargins(20, 15, 20, 15)
        
        # Row 1: Judul (Kiri) - Nama Wallet (Kanan Biru)
        row1 = QHBoxLayout()
        lbl_name = QLabel(title)
        lbl_name.setStyleSheet("font-weight: bold; font-size: 16px; color: black; border: none;")
        
        lbl_wallet = QLabel(wallet_name)
        lbl_wallet.setStyleSheet("font-weight: bold; font-size: 14px; color: #5873E8; border: none;")
        
        row1.addWidget(lbl_name)
        row1.addStretch()
        row1.addWidget(lbl_wallet)
        
        # Row 2: Progress Bar Besar
        pbar = QProgressBar()
        # Cap percent at 100 if current exceeds target
        percent = int((display_current / target) * 100) if target > 0 else 0
        percent = min(percent, 100)  # Ensure max 100%
        pbar.setValue(percent)
        pbar.setFixedHeight(8)
        pbar.setTextVisible(False)
        pbar.setStyleSheet("""
            QProgressBar {
                background-color: #E0E4F5;
                border-radius: 4px;
                border: none;
            }
            QProgressBar::chunk {
                background-color: #5873E8;
                border-radius: 4px;
            }
        """)
        
        # Row 3: Text Info (Terkumpul X dari Y)
        row3 = QHBoxLayout()
        # Format text: "Terkumpul" (biru muda/abu) "Rp..." (Biru Tua)
        # Show capped amount if balance exceeds target
        info_text = f"Terkumpul Rp{display_current:,.0f}".replace(",", ".") + f" dari <b>Rp{target:,.0f}</b>".replace(",", ".")
        lbl_info = QLabel(info_text)
        lbl_info.setAlignment(Qt.AlignmentFlag.AlignCenter)
        lbl_info.setStyleSheet("color: #5873E8; font-size: 12px; border: none;")
        
        row3.addStretch()
        row3.addWidget(lbl_info)
        row3.addStretch()

        card_layout.addLayout(row1)
        card_layout.addWidget(pbar)
        card_layout.addLayout(row3)
        
        wrapper_layout.addWidget(card)
        
        # Tombol Hapus (Di luar frame kartu, aligned right)
        btn_delete = QPushButton(" Hapus target")
        btn_delete.setIcon(QIcon.fromTheme("trash", QIcon("img/Trash.svg"))) 
        btn_delete.setCursor(Qt.CursorShape.PointingHandCursor)
        btn_delete.setStyleSheet("""
            QPushButton {
                background-color: transparent;
                color: black;
                font-size: 12px;
                border: none;
                text-align: right;
            }
            QPushButton:hover { color: red; }
        """)
        # Connect delete button to handler
        if target_id and account_id:
            btn_delete.clicked.connect(lambda checked, tid=target_id, aid=account_id, name=title: 
                                       self._on_delete_target(tid, aid, name))
        
        # Layout untuk tombol hapus agar di kanan
        bottom_row = QHBoxLayout()
        bottom_row.addStretch()
        bottom_row.addWidget(btn_delete)
        
        wrapper_layout.addLayout(bottom_row)
        
        return container

    def _on_target_double_clicked(self, target_id, target_name, target_amount, account_id, deadline=None):
        """Handle double-click on target card to edit"""
        if not target_id:
            return
        
        # Prepare target data dict
        target_data = {
            'target_id': target_id,
            'target_name': target_name,
            'target_amount': target_amount,
            'account_id': account_id,
            'deadline': deadline
        }
        
        # Emit signal with target id and data
        self.target_edit_requested.emit(target_id, target_data)
    
    def _on_delete_target(self, target_id: str, account_id: str, target_name: str):
        """Handle delete target button click"""
        # Show custom confirmation dialog
        dialog = KonfirmasiHapusTargetDialog(self, target_name)
        
        if dialog.exec() != QDialog.DialogCode.Accepted:
            return
        
        # Delete target from database
        if self.target_controller:
            success = self.target_controller.delete_target(target_id, account_id)
            if success:
                # Refresh the view to remove the deleted target
                self.refresh()
                # Emit signal to notify MainWindow
                self.target_deleted.emit()
            else:
                # Show error message
                error_msg = QMessageBox(self)
                error_msg.setWindowTitle("Kesalahan")
                error_msg.setText("Gagal menghapus target. Silakan coba lagi.")
                error_msg.setIcon(QMessageBox.Icon.Critical)
                error_msg.setStyleSheet("""
                    QMessageBox { background-color: white; }
                    QLabel { color: black; }
                    QPushButton { background-color: #5873E8; color: white; border-radius: 5px; padding: 5px 15px; }
                """)
                error_msg.exec()

# --- Contoh Eksekusi (Untuk Pengujian Mandiri) ---
if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = ProgressView()
    window.show()
    sys.exit(app.exec())
