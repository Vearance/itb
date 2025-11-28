"""

Cara jalankan dari root:
    python src/test_history.py
"""

from PyQt6.QtWidgets import QApplication
from models.User import User
from views.HistoryView import HistoryView
import sys


def test_history_view():
    """Test HistoryView saja"""
    app = QApplication(sys.argv)
    
    # Buat user dummy
    dummy_user = User("user_001", "Rafi Akbar", "password123")
    
    # Buat HistoryView
    history_view = HistoryView(dummy_user)
    
    # Connect signals untuk testing
    history_view.transaction_clicked.connect(
        lambda trans_id: print(f"✓ [History] Transaction clicked: {trans_id}")
    )
    history_view.report_requested.connect(
        lambda start, end: print(f"✓ [History] Report requested: {start} to {end}")
    )
    
    # Tampilkan window
    history_view.show()
    
    sys.exit(app.exec())


if __name__ == "__main__":
    test_history_view()