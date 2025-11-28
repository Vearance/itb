from PyQt6.QtWidgets import (
    QWidget, QLabel, QPushButton, QVBoxLayout, QHBoxLayout, 
    QFrame, QScrollArea, QSizePolicy, QGraphicsDropShadowEffect, QButtonGroup
)
from PyQt6.QtCore import Qt, pyqtSignal, QSize, QPointF
from PyQt6.QtGui import QPixmap, QIcon, QPainter, QPen, QColor, QBrush, QPainterPath, QLinearGradient
from models.User import User
import matplotlib
matplotlib.use('QtAgg')
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.figure import Figure
import numpy as np
from datetime import datetime, timedelta
from collections import defaultdict


class TransactionChart(FigureCanvas):
    """Dynamic chart that displays transaction data based on time interval."""
    
    def __init__(self, transaction_controller=None, account_controller=None, user=None):
        self.fig = Figure(figsize=(5, 3), dpi=100)
        self.fig.patch.set_facecolor('none')
        super().__init__(self.fig)
        
        self.transaction_controller = transaction_controller
        self.account_controller = account_controller
        self.user = user
        self.current_interval = "Mingguan"  # Default interval
        
        self.setMinimumHeight(250)
        self.setStyleSheet("background-color: transparent;")
        self.setObjectName("chartTransaksi")
        
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
        self.ax.bar(x - width/2, income_values, width, 
                    label='Pemasukan', color='#2ECC71', alpha=0.8)
        self.ax.bar(x + width/2, expense_values, width, 
                    label='Pengeluaran', color='#E74C3C', alpha=0.8)
        
        # Also plot net savings as a line
        net_values = [inc - exp for inc, exp in zip(income_values, expense_values)]
        self.ax.plot(x, net_values, color='#5873E8', linewidth=2, 
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
        self.ax.grid(True, axis='y', linestyle='--', alpha=0.5, color='#E0E0E0')
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
        self.ax.legend(loc='upper left', fontsize=7, framealpha=0.9)
        
        self.fig.tight_layout()
        self.draw()
    
    def _get_transaction_data(self, interval: str) -> dict:
        """Fetch and aggregate transaction data based on interval."""
        if not self.transaction_controller or not self.account_controller or not self.user:
            return {"labels": [], "income": [], "expense": []}
        
        # Get all user accounts
        accounts = self.account_controller.get_user_accounts(self.user.user_id)
        if not accounts:
            return {"labels": [], "income": [], "expense": []}
        
        # Collect all transactions from all accounts
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
        elif interval == "Semua":
            return self._aggregate_all(all_transactions)
        
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
    
    def _aggregate_all(self, transactions: list) -> dict:
        """Aggregate all transactions by month (all time)."""
        if not transactions:
            return {"labels": [], "income": [], "expense": []}
        
        # Get all unique months from transactions
        months_set = set()
        for trans in transactions:
            trans_date = trans["date"]
            months_set.add((trans_date.year, trans_date.month))
        
        months = sorted(list(months_set))[-12:]  # Show last 12 months max
        
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

class Homepage(QWidget):
    logout_requested = pyqtSignal()
    
    def __init__(self, user: User, account_controller=None, transaction_controller=None, 
                 target_controller=None, report_controller=None):
        super().__init__()
        self.user = user
        self.account_controller = account_controller
        self.transaction_controller = transaction_controller
        self.target_controller = target_controller
        self.report_controller = report_controller
        self.current_interval = "Mingguan"  # Default interval
        self.chart = None  # Will hold reference to chart widget
        self.interval_buttons = {}  # Will hold references to interval buttons
        self.initUI()
    
    def refresh(self):
        """Refresh homepage with updated data"""
        # Update balance card
        if hasattr(self, 'balance_amount_label'):
            total_balance = self.get_total_balance()
            self.balance_amount_label.setText(f"Rp{total_balance:,.0f}".replace(",", "."))
        
        # Update progress section
        if hasattr(self, 'progress_bubble_val'):
            total_savings = self.get_total_savings()
            self.progress_bubble_val.setText(f"Rp{total_savings:,.0f}".replace(",", "."))
            
            # Update progress bar
            total_target_amount = self.get_total_target_amount()
            if total_target_amount > 0:
                filled_bars = int((total_savings / total_target_amount) * 45)
            else:
                filled_bars = 0
            
            # Recreate progress bar
            if hasattr(self, 'bar_layout'):
                while self.bar_layout.count():
                    child = self.bar_layout.takeAt(0)
                    if child.widget():
                        child.widget().deleteLater()
                
                total_bars = 45
                for i in range(total_bars):
                    line = QFrame()
                    line.setFixedSize(6, 25)
                    if i < filled_bars:
                        line.setStyleSheet("background-color: #5873E8; border-radius: 3px;")
                    else:
                        line.setStyleSheet("background-color: #E0E4F5; border-radius: 3px;")
                    self.bar_layout.addWidget(line, 0, Qt.AlignmentFlag.AlignCenter)
            
            # Update progress labels
            if hasattr(self, 'lbl_current_savings'):
                self.lbl_current_savings.setText(f"Rp{total_savings:,.0f}".replace(",", "."))
            if hasattr(self, 'lbl_target_amount'):
                self.lbl_target_amount.setText(f"Rp{total_target_amount:,.0f}".replace(",", "."))
        
        # Update chart
        if hasattr(self, 'chart') and self.chart:
            self.chart.update_chart(self.current_interval)
    
    def initUI(self):
        self.setStyleSheet("background-color: white;")
        
        main_layout = QHBoxLayout()
        main_layout.setContentsMargins(0, 0, 0, 0)
        main_layout.setSpacing(0)
        self.setLayout(main_layout)
        
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
        
        header = self.createHeader()
        self.content_layout.addLayout(header)
        
        cards_layout = QVBoxLayout()
        cards_layout.setSpacing(15)
        balance_card = self.createBalanceCard() 
        cards_layout.addWidget(balance_card)
        self.content_layout.addLayout(cards_layout)
        
        progress_section = self.createProgressSection()
        self.content_layout.addWidget(progress_section)

        transaction_section = self.createTransactionSection()
        self.content_layout.addWidget(transaction_section)
        
        self.content_layout.addStretch() 
        
        scroll_area.setWidget(content_widget)
        main_layout.addWidget(scroll_area)



    def createHeader(self):
        layout = QHBoxLayout()
        lbWelcome = QLabel(f"Selamat datang, <b>{self.user.username}</b>")
        lbWelcome.setObjectName("lbWelcome")
        lbWelcome.setStyleSheet("font-size: 24px; color: black;")
        lbWelcome.setTextFormat(Qt.TextFormat.RichText)
        
        layout.addWidget(lbWelcome)
        layout.addStretch()
        return layout

    def createBalanceCard(self):
        card = QFrame()
        card.setStyleSheet("background-color: #F8F9FE; border: 1px solid #5873E8; border-radius: 12px;")
        card.setFixedHeight(100)
        
        layout = QHBoxLayout(card)
        layout.setContentsMargins(25, 20, 25, 20)
        
        icon_label = QLabel()
        icon_label.setFixedSize(48, 48)
        icon_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        icon_label.setStyleSheet("background: transparent; border: none;")
        
        icon = QIcon("img/Saldo.svg")
        if not icon.isNull():
            icon_label.setPixmap(icon.pixmap(48, 48))
        else:
            icon_label.setText("Rp") 
            icon_label.setStyleSheet("color: #5873E8; font-size: 24px; font-weight: bold; border: none;")

        text_layout = QVBoxLayout()
        text_layout.setSpacing(5)
        
        title = QLabel("Total saldo kamu")
        title.setStyleSheet("color: #5873E8; font-weight: bold; font-size: 14px; border: none; background: transparent;")
        
        # Get real balance from accounts
        balance = self.get_total_balance()
        self.balance_amount_label = QLabel(f"Rp{balance:,.0f}".replace(",", "."))
        self.balance_amount_label.setStyleSheet("color: #5873E8; font-size: 24px; font-weight: bold; border: none; background: transparent;")
        
        text_layout.addWidget(title)
        text_layout.addWidget(self.balance_amount_label)
        layout.addWidget(icon_label)
        layout.addSpacing(20)
        layout.addLayout(text_layout)
        layout.addStretch()
        return card

    def createSavingsCard(self):
        card = QFrame()
        card.setStyleSheet("background-color: #5873E8; border-radius: 12px;")
        card.setFixedHeight(100)
        
        layout = QHBoxLayout(card)
        layout.setContentsMargins(25, 20, 25, 20)
        
        icon_label = QLabel()
        icon_label.setFixedSize(48, 48)
        icon_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        icon_label.setStyleSheet("background: transparent; border: none;")
        
        icon = QIcon("img/MoneyTree.svg")
        if not icon.isNull():
            icon_label.setPixmap(icon.pixmap(48, 48))
        else:
            icon_label.setText("Sv")
            icon_label.setStyleSheet("color: white; font-size: 24px; font-weight: bold; border: none;")

        text_layout = QVBoxLayout()
        text_layout.setSpacing(5)
        
        title = QLabel("Total tabungan kamu saat ini")
        title.setStyleSheet("color: white; font-size: 14px; background: transparent;")
        
        # Get real savings from targets
        savings = self.get_total_savings()
        amount = QLabel(f"Rp{savings:,.0f}".replace(",", "."))
        amount.setStyleSheet("color: white; font-size: 24px; font-weight: bold; background: transparent;")
        
        text_layout.addWidget(title)
        text_layout.addWidget(amount)
        layout.addWidget(icon_label)
        layout.addSpacing(20)
        layout.addLayout(text_layout)
        layout.addStretch()
        return card

    def createProgressSection(self):
        section = QFrame()
        section.setStyleSheet("background-color: white; border: 1px solid #5873E8; border-radius: 15px;")
        
        layout = QVBoxLayout(section)
        layout.setContentsMargins(30, 30, 30, 30)
        layout.setSpacing(20)
        
        lbProgress = QLabel("Progress semua tabungan kamu")
        lbProgress.setObjectName("lbProgress")
        lbProgress.setStyleSheet("font-size: 18px; font-weight: bold; color: black; border: none;")
        lbProgress.setAlignment(Qt.AlignmentFlag.AlignCenter)
        
        # Get real savings data
        total_savings = self.get_total_savings()
        
        bubble_layout = QVBoxLayout()
        self.progress_bubble_val = QLabel(f"Rp{total_savings:,.0f}".replace(",", "."))
        self.progress_bubble_val.setFixedSize(120, 30)
        self.progress_bubble_val.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.progress_bubble_val.setStyleSheet("background-color: #5873E8; color: white; border-radius: 15px; font-weight: bold;")
        bubble_layout.addWidget(self.progress_bubble_val, 0, Qt.AlignmentFlag.AlignCenter)
        
        ProgressBar = QFrame()
        ProgressBar.setObjectName("ProgressBar")
        ProgressBar.setFixedHeight(40)
        ProgressBar.setStyleSheet("border: none;")
        self.bar_layout = QHBoxLayout(ProgressBar)
        self.bar_layout.setSpacing(4) 
        self.bar_layout.setContentsMargins(0, 0, 0, 0)
        
        # Calculate progress based on real targets
        total_target_amount = self.get_total_target_amount()
        if total_target_amount > 0:
            filled_bars = int((total_savings / total_target_amount) * 45)
        else:
            filled_bars = 0
        
        total_bars = 45
        for i in range(total_bars):
            line = QFrame()
            line.setFixedSize(6, 25) 
            if i < filled_bars:
                line.setStyleSheet("background-color: #5873E8; border-radius: 3px;")
            else:
                line.setStyleSheet("background-color: #E0E4F5; border-radius: 3px;")
            self.bar_layout.addWidget(line, 0, Qt.AlignmentFlag.AlignCenter)
            
        labels_layout = QHBoxLayout()
        self.lbl_current_savings = QLabel(f"Rp{total_savings:,.0f}".replace(",", "."))
        self.lbl_current_savings.setStyleSheet("color: #5873E8; border: none;")
        self.lbl_target_amount = QLabel(f"Rp{total_target_amount:,.0f}".replace(",", "."))
        self.lbl_target_amount.setStyleSheet("color: #5873E8; font-weight: bold; border: none;")
        labels_layout.addWidget(self.lbl_current_savings)
        labels_layout.addStretch()
        labels_layout.addWidget(self.lbl_target_amount)
        
        btnProgressDetail = QPushButton("Lihat progress dan target menabung")
        btnProgressDetail.setObjectName("btnProgressDetail")
        btnProgressDetail.setFixedHeight(45)
        btnProgressDetail.setCursor(Qt.CursorShape.PointingHandCursor)
        btnProgressDetail.setStyleSheet("""
            QPushButton { background-color: #5873E8; color: white; font-weight: bold; border-radius: 8px; font-size: 14px; }
            QPushButton:hover { background-color: #4A63D0; }
        """)
        btnProgressDetail.clicked.connect(lambda: print("Navigate to progress"))
        
        layout.addWidget(lbProgress)
        layout.addLayout(bubble_layout)
        layout.addWidget(ProgressBar)
        layout.addLayout(labels_layout)
        layout.addSpacing(10)
        layout.addWidget(btnProgressDetail)
        return section

    def createTransactionSection(self):
        section = QFrame()
        section.setStyleSheet("background-color: white; border-radius: 15px;")
        
        main_layout = QVBoxLayout(section)
        main_layout.setContentsMargins(0, 20, 0, 0)
        main_layout.setSpacing(20)
        
        title = QLabel("Total Transaksi Keseluruhan")
        title.setStyleSheet("font-size: 20px; font-weight: bold; color: black;")
        title.setAlignment(Qt.AlignmentFlag.AlignCenter)
        main_layout.addWidget(title)
        
        filter_layout = QHBoxLayout()
        filter_layout.setSpacing(15)
        filter_layout.addStretch()
        
        filters = ["Harian", "Mingguan", "Bulanan"]
        for f in filters:
            btnFilterPeriode = QPushButton(f)
            btnFilterPeriode.setObjectName(f"btnFilterPeriode_{f}")
            btnFilterPeriode.setFixedSize(120, 40)
            btnFilterPeriode.setCursor(Qt.CursorShape.PointingHandCursor)
            if f == self.current_interval:
                btnFilterPeriode.setStyleSheet("""
                    background-color: #5873E8; color: white; border-radius: 10px; font-weight: bold; font-size: 14px;
                """)
            else:
                btnFilterPeriode.setStyleSheet("""
                    QPushButton { 
                        background-color: white; color: #5873E8; border: 1px solid #5873E8; border-radius: 10px; font-size: 14px; font-weight: bold;
                    }
                    QPushButton:hover { background-color: #F0F2FF; }
                """)
            btnFilterPeriode.clicked.connect(lambda checked, interval=f: self._on_filter_changed(interval))
            self.interval_buttons[f] = btnFilterPeriode
            filter_layout.addWidget(btnFilterPeriode)
            
        filter_layout.addStretch()
        main_layout.addLayout(filter_layout)
        
        content_split = QHBoxLayout()
        content_split.setSpacing(30)
        content_split.setContentsMargins(20, 10, 20, 10)
        
        self.stats_layout = QVBoxLayout()
        self.stats_layout.setContentsMargins(0, 0, 0, 0)
        self.stats_layout.setSpacing(15)
        
        # Get real transaction data
        summary = self.get_transaction_summary()
        income = summary["income"]
        expense = summary["expense"]
        savings = summary["savings"]
        
        self.income_card = self.createStatCard("Pemasukan", f"+Rp{income:,.0f}".replace(",", "."), "#2ECC71")
        self.expense_card = self.createStatCard("Pengeluaran", f"-Rp{expense:,.0f}".replace(",", "."), "#E74C3C")
        self.savings_card = self.createStatCard("Tabungan", f"+Rp{savings:,.0f}".replace(",", "."), "#5873E8")
        
        self.stats_layout.addWidget(self.income_card)
        self.stats_layout.addWidget(self.expense_card)
        self.stats_layout.addWidget(self.savings_card)
        
        btnLihatSelengkapnya = QPushButton("Lihat selengkapnya >")
        btnLihatSelengkapnya.setObjectName("btnLihatSelengkapnya")
        btnLihatSelengkapnya.setCursor(Qt.CursorShape.PointingHandCursor)
        btnLihatSelengkapnya.setStyleSheet("""
            QPushButton {
                color: #5873E8; 
                font-size: 12px; 
                text-align: left; 
                border: none; 
                background: transparent;
            }
            QPushButton:hover { text-decoration: underline; }
        """)
        self.stats_layout.addWidget(btnLihatSelengkapnya)
        self.stats_layout.addStretch()
        
        # Create dynamic chart
        self.chart = TransactionChart(self.transaction_controller, self.account_controller, self.user)
        self.chart.update_chart(self.current_interval)
        
        content_split.addLayout(self.stats_layout)
        content_split.addWidget(self.chart, 1)
        
        main_layout.addLayout(content_split)
        
        return section
    
    def _on_filter_changed(self, interval: str):
        """Handle interval button clicks."""
        self.current_interval = interval
        
        # Update button styles
        for name, btn in self.interval_buttons.items():
            if name == interval:
                btn.setStyleSheet("""
                    background-color: #5873E8; color: white; border-radius: 10px; font-weight: bold; font-size: 14px;
                """)
            else:
                btn.setStyleSheet("""
                    QPushButton { 
                        background-color: white; color: #5873E8; border: 1px solid #5873E8; border-radius: 10px; font-size: 14px; font-weight: bold;
                    }
                    QPushButton:hover { background-color: #F0F2FF; }
                """)
        
        # Update chart
        if self.chart:
            self.chart.update_chart(interval)

    def createStatCard(self, title_text, amount_text, color_hex):
        """Helper untuk membuat kartu kecil di kiri grafik"""
        card = QFrame()
        card.setFixedSize(180, 70)
        card.setStyleSheet(f"""
            QFrame {{
                background-color: white;
                border: 1px solid {color_hex};
                border-radius: 12px;
            }}
        """)
        
        layout = QVBoxLayout(card)
        layout.setContentsMargins(15, 10, 15, 10)
        layout.setSpacing(2)
        
        lblStatistik = QLabel(title_text)
        lblStatistik.setObjectName("lblStatistik")
        lblStatistik.setStyleSheet(f"color: {color_hex}; font-size: 12px; border: none;")
        
        lbl_amount = QLabel(amount_text)
        lbl_amount.setStyleSheet(f"color: {color_hex}; font-size: 14px; font-weight: bold; border: none;")
        
        layout.addWidget(lblStatistik)
        layout.addWidget(lbl_amount)
        
        return card

    # Helper methods to fetch real data from controllers
    def get_total_balance(self) -> float:
        """Get total balance across all accounts for current user"""
        if not self.account_controller:
            return 0.0
        accounts = self.account_controller.get_user_accounts(self.user.user_id)
        return sum(acc.get_balance() for acc in accounts)

    def get_total_savings(self) -> float:
        """Get total savings from targets across all accounts"""
        if not self.target_controller or not self.account_controller:
            return 0.0
        accounts = self.account_controller.get_user_accounts(self.user.user_id)
        total_savings = 0.0
        for account in accounts:
            targets = self.target_controller.get_account_targets(account.account_id)
            total_savings += sum(t.current_amount for t in targets)
        return total_savings

    def get_total_target_amount(self) -> float:
        """Get total target amount across all accounts"""
        if not self.target_controller or not self.account_controller:
            return 0.0
        accounts = self.account_controller.get_user_accounts(self.user.user_id)
        total_target = 0.0
        for account in accounts:
            targets = self.target_controller.get_account_targets(account.account_id)
            total_target += sum(t.target_amount for t in targets)
        return total_target

    def get_transaction_summary(self) -> dict:
        """Get transaction summary for current user"""
        if not self.transaction_controller or not self.account_controller:
            return {"income": 0.0, "expense": 0.0, "savings": 0.0}
        
        accounts = self.account_controller.get_user_accounts(self.user.user_id)
        total_income = 0.0
        total_expense = 0.0
        
        for account in accounts:
            transactions = self.transaction_controller.get_account_transactions(account.account_id)
            for trans in transactions:
                if trans.is_income():
                    total_income += trans.amount
                else:
                    total_expense += trans.amount
        
        
        return {
            "income": total_income,
            "expense": total_expense,
            "savings": total_income - total_expense
        }


