from typing import List, Optional, Dict
from models.Transaction import Transaction, TransactionType
from models.Account import Account
from database import Database
from datetime import datetime
from calendar import monthrange


class TransactionController:
    def __init__(self, db: Database = None, target_controller=None):
        """Initialize TransactionController"""
        self.db = db
        self.target_controller = target_controller
    
    def set_target_controller(self, target_controller):
        """Set the target controller for updating target progress."""
        self.target_controller = target_controller

    def add_transaction(
        self,
        transaction_id: str,
        account_id: str,
        amount: float,
        transaction_type: TransactionType,
        description: str,
        account: Account
    ) -> tuple:
        """
        Returns:
            tuple: (bool: True if transaction added successfully, list: achieved targets or empty list)
        """
        if not self.db:
            return False, []

        # Check if transaction already exists
        existing = self.db.fetch_one(
            "SELECT * FROM transactions WHERE transaction_id = ?",
            (transaction_id,)
        )
        if existing:
            return False, []

        # Update account balance in memory
        if transaction_type == TransactionType.INCOME:
            if not account.add_balance(amount):
                return False, []
        else:  # expense
            if not account.subtract_balance(amount):
                return False, []

        # Create transaction in database
        transaction = Transaction(
            transaction_id,
            account_id,
            amount,
            transaction_type,
            description
        )

        success = self.db.execute(
            "INSERT INTO transactions (transaction_id, account_id, amount, type, description, date) VALUES (?, ?, ?, ?, ?, ?)",
            (transaction.transaction_id, transaction.account_id, transaction.amount, transaction.transaction_type.value, transaction.description, transaction.date.isoformat())
        )

        achieved_targets = []
        # Update account balance in database
        if success:
            self.db.execute(
                "UPDATE accounts SET balance = ? WHERE account_id = ?",
                (account.get_balance(), account_id)
            )
            
            # Update target progress based on transaction type
            if self.target_controller:
                if transaction_type == TransactionType.INCOME:
                    achieved_targets = self._update_account_targets(account_id, amount) or []
                else:  # EXPENSE - subtract from target progress
                    self._subtract_from_account_targets(account_id, amount)

        return success, achieved_targets
    
    def _update_account_targets(self, account_id: str, amount: float):
        """Update all non-archived targets for an account with the income amount."""
        if not self.target_controller:
            return
        
        # Get all targets for this account
        targets = self.target_controller.get_account_targets(account_id)
        
        achieved_targets = []
        for target in targets:
            if not target.is_archived:
                # Check if target will be achieved after adding this amount
                new_current = target.current_amount + amount
                if new_current >= target.target_amount:
                    # Target will be achieved, record it before reset
                    achieved_targets.append(target)
                
                # Add the income amount to the target's progress
                # Note: add_progress will reset to 0 after achieving
                self.target_controller.add_progress(target.target_id, amount)
        
        # Return list of newly achieved targets for notification
        return achieved_targets
    
    def _subtract_from_account_targets(self, account_id: str, amount: float):
        """Subtract from all non-archived targets for an account (when income is deleted)."""
        if not self.target_controller or not self.db:
            return
        
        # Get all targets for this account
        targets = self.target_controller.get_account_targets(account_id)
        
        for target in targets:
            if not target.is_archived:
                # Subtract the amount from target's progress (but not below 0)
                new_amount = max(0, target.current_amount - amount)
                self.db.execute(
                    "UPDATE targets SET current_amount = ? WHERE target_id = ?",
                    (new_amount, target.target_id)
                )

    def get_transaction(self, transaction_id: str) -> Optional[Transaction]:
        """
        Args:
            transaction_id: Transaction ID
        Returns:
            Transaction or None if not found
        """
        if not self.db:
            return None
        transaction_row = self.db.fetch_one(
            "SELECT * FROM transactions WHERE transaction_id = ?",
            (transaction_id,)
        )

        if transaction_row:
            transaction = Transaction(
                transaction_row['transaction_id'],
                transaction_row['account_id'],
                transaction_row['amount'],
                TransactionType(transaction_row['type']),
                transaction_row['description'],
                datetime.fromisoformat(transaction_row['date'])
            )
            return transaction
        return None

    def get_account_transactions(self, account_id: str) -> List[Transaction]:
        """
        Args:
            account_id: Account ID

        Returns:
            List of transactions for the account
        """
        if not self.db:
            return []

        transaction_rows = self.db.fetch_all(
            "SELECT * FROM transactions WHERE account_id = ? ORDER BY date DESC",
            (account_id,)
        )

        transactions = []
        for row in transaction_rows:
            transaction = Transaction(
                row['transaction_id'],
                row['account_id'],
                row['amount'],
                TransactionType(row['type']),
                row['description'],
                datetime.fromisoformat(row['date'])
            )
            transactions.append(transaction)

        return transactions

    def delete_transaction(
        self,
        transaction_id: str,
        account: Account
    ) -> bool:
        """
        Args:
            transaction_id: Transaction ID to delete
            account: Associated account object
        Returns:
            bool: True if deletion successful
        """
        if not self.db:
            return False

        transaction = self.get_transaction(transaction_id)
        if not transaction:
            return False

        # Reverse the transaction effect on account balance
        if transaction.is_income():
            account.subtract_balance(transaction.amount)
            # Also subtract from target progress
            if self.target_controller:
                self._subtract_from_account_targets(account.account_id, transaction.amount)
        else:
            account.add_balance(transaction.amount)
            # Also add back to target progress (reverse the expense subtraction)
            if self.target_controller:
                self._update_account_targets(account.account_id, transaction.amount)

        # Delete from database
        return self.db.execute(
            """
            UPDATE accounts SET balance = ? WHERE account_id = ?
            """,
            (account.get_balance(), account.account_id)
        ) and self.db.execute(
            """
            DELETE FROM transactions WHERE transaction_id = ?
            """,
            (transaction_id,)
        )

    def update_transaction(
        self,
        transaction_id: str,
        new_amount: float,
        new_type: TransactionType,
        new_description: str,
        account: Account
    ) -> bool:
        """
        Update an existing transaction.
        Args:
            transaction_id: Transaction ID to update
            new_amount: New transaction amount
            new_type: New transaction type
            new_description: New description
            account: Associated account object
        Returns:
            bool: True if update successful
        """
        if not self.db:
            return False

        old_transaction = self.get_transaction(transaction_id)
        if not old_transaction:
            return False

        # Reverse the old transaction effect on account balance and targets
        if old_transaction.is_income():
            account.subtract_balance(old_transaction.amount)
            if self.target_controller:
                self._subtract_from_account_targets(account.account_id, old_transaction.amount)
        else:
            account.add_balance(old_transaction.amount)
            if self.target_controller:
                self._update_account_targets(account.account_id, old_transaction.amount)

        # Apply the new transaction effect
        if new_type == TransactionType.INCOME:
            if not account.add_balance(new_amount):
                # Rollback: restore old balance
                if old_transaction.is_income():
                    account.add_balance(old_transaction.amount)
                else:
                    account.subtract_balance(old_transaction.amount)
                return False
        else:  # expense
            if not account.subtract_balance(new_amount):
                # Rollback: restore old balance
                if old_transaction.is_income():
                    account.add_balance(old_transaction.amount)
                else:
                    account.subtract_balance(old_transaction.amount)
                return False

        # Update transaction in database
        success = self.db.execute(
            """
            UPDATE transactions 
            SET amount = ?, type = ?, description = ?
            WHERE transaction_id = ?
            """,
            (new_amount, new_type.value, new_description, transaction_id)
        )

        if success:
            # Update account balance in database
            self.db.execute(
                "UPDATE accounts SET balance = ? WHERE account_id = ?",
                (account.get_balance(), account.account_id)
            )
            
            # Update target progress based on new transaction type
            if self.target_controller:
                if new_type == TransactionType.INCOME:
                    self._update_account_targets(account.account_id, new_amount)
                else:
                    self._subtract_from_account_targets(account.account_id, new_amount)

        return success

    def get_transaction_summary(
        self,
        account_id: str,
        start_date: datetime,
        end_date: datetime
    ) -> Dict:
        """
        Args:
            account_id: Account ID
            start_date: Start date
            end_date: End date   
        Returns:
            Dict with total_income, total_expense, net_amount, transaction_count
        """
        transactions = self.get_account_transactions(account_id)

        # Filter by date range
        transactions = [
            trans for trans in transactions
            if start_date <= trans.date <= end_date
        ]

        total_income = 0.0
        total_expense = 0.0

        for trans in transactions:
            if trans.is_income():
                total_income += trans.amount
            else:
                total_expense += trans.amount

        net_amount = total_income - total_expense

        return {
            "total_income": total_income,
            "total_expense": total_expense,
            "net_amount": net_amount,
            "transaction_count": len(transactions)
        }

    def get_transactions_by_date_range(
        self,
        account_id: str,
        start_date: datetime,
        end_date: datetime
    ) -> List[Transaction]:
        """
        Args:
            account_id: Account ID
            start_date: Start date
            end_date: End date
        Returns:
            List of transactions within the date range
        """
        transactions = self.get_account_transactions(account_id)
        return [
            trans for trans in transactions
            if start_date <= trans.date <= end_date
        ]

    def get_monthly_summary(
        self,
        account_id: str,
        year: int,
        month: int
    ) -> Dict:
        """
        Args:
            account_id: Account ID
            year: Year (e.g., 2025)
            month: Month (1-12)
        Returns:
            Dict with total_income, total_expense, net_amount, transaction_count
        """
        # Calculate first and last day of month
        last_day = monthrange(year, month)[1]

        start_date = datetime(year, month, 1)
        end_date = datetime(year, month, last_day, 23, 59, 59)

        return self.get_transaction_summary(account_id, start_date, end_date)

    def get_monthly_transactions(
        self,
        account_id: str,
        year: int,
        month: int
    ) -> List[Transaction]:
        """
        Args:
            account_id: Account ID
            year: Year (e.g., 2025)
            month: Month (1-12)
        Returns:
            List of transactions in the month
        """
        # Calculate first and last day of month
        last_day = monthrange(year, month)[1]

        start_date = datetime(year, month, 1)
        end_date = datetime(year, month, last_day, 23, 59, 59)

        return self.get_transactions_by_date_range(account_id, start_date, end_date)
