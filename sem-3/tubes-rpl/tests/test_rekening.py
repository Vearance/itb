import sys
from PyQt6.QtWidgets import QApplication
from models.User import User
from views.RekeningView import RekeningViews

def main():
    app = QApplication(sys.argv)
    
    # Buat user dummy untuk testing
    dummy_user = User("user_001", "Rafi Akbar", "password123")
    
    # Buat RekeningView
    rekening_view = RekeningViews(dummy_user)
    
    # Connect signals untuk testing 
    rekening_view.add_account_requested.connect(
        lambda name: print(f"✓ Add account: {name}")
    )
    rekening_view.delete_account_requested.connect(
        lambda name: print(f"✓ Delete account: {name}")
    )
    rekening_view.transaction_requested.connect(
        lambda name: print(f"✓ Transaction for: {name}")
    )
    rekening_view.view_history_requested.connect(
        lambda name: print(f"✓ View history for: {name}")
    )
    
    # Tampilkan window
    rekening_view.show()
    
    sys.exit(app.exec())

if __name__ == "__main__":
    main()