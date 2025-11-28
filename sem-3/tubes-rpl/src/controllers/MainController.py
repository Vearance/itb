from typing import Optional
import sys
from PyQt6.QtWidgets import QApplication

from database import Database
from controllers.LoginController import LoginController
from controllers.AccountController import AccountController
from controllers.TransactionController import TransactionController
from controllers.TargetController import TargetController
from controllers.ReportController import ReportController


class MainController:
    """Central application controller.

    Responsibilities:
    - Initialize Database and application controllers
    - Provide accessors for controllers
    - Start the Qt application and show initial view
    """

    def __init__(self, db_path: str = "tabungin.db"):
        self.db_path = db_path
        self.db: Optional[Database] = None

        # controllers (populated after DB init)
        self.login_controller: Optional[LoginController] = None
        self.account_controller: Optional[AccountController] = None
        self.transaction_controller: Optional[TransactionController] = None
        self.target_controller: Optional[TargetController] = None
        self.report_controller: Optional[ReportController] = None

    def init(self) -> bool:
        """Initialize database and controllers. Returns True on success."""
        self.db = Database(self.db_path)
        if not self.db.connect():
            print("Failed to connect to database")
            return False

        if not self.db.init_tables():
            print("Failed to initialize tables")
            return False

        # Create controllers
        self.login_controller = LoginController(self.db)
        self.account_controller = AccountController(self.db)
        self.target_controller = TargetController(self.db)
        self.transaction_controller = TransactionController(self.db, self.target_controller)
        self.report_controller = ReportController(self.db)

        return True

    def start(self) -> int:
        """Start the Qt application and show the main window.

        Returns the QApplication exit code.
        """
        # Lazy-init database & controllers if not already done
        if not self.db:
            ok = self.init()
            if not ok:
                return 1

        # Import here to avoid Qt imports at module import time when not running GUI
        from views.MainWindow import MainWindow

        app = QApplication(sys.argv)

        main_window = MainWindow(
            login_controller=self.login_controller,
            account_controller=self.account_controller,
            transaction_controller=self.transaction_controller,
            target_controller=self.target_controller,
            report_controller=self.report_controller
        )
        main_window.show()

        return app.exec()

    # Accessors for controllers (small helpers)
    def get_login_controller(self) -> Optional[LoginController]:
        return self.login_controller

    def get_account_controller(self) -> Optional[AccountController]:
        return self.account_controller

    def get_transaction_controller(self) -> Optional[TransactionController]:
        return self.transaction_controller

    def get_target_controller(self) -> Optional[TargetController]:
        return self.target_controller

    def get_report_controller(self) -> Optional[ReportController]:
        return self.report_controller
