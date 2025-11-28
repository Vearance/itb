from typing import List, Optional
from models.Account import Account
from database import Database
from datetime import datetime


class AccountController:  
    def __init__(self, db: Database = None):
        """Initialize AccountController"""
        self.db = db

    def create_account(
        self,
        account_id: str,
        account_name: str,
        user_id: str,
        initial_balance: float = 0.0
    ) -> bool:
        """
        Returns:
            bool: True if account created successfully
        """
        if not self.db:
            return False

        # Check if account already exists
        existing_account = self.db.fetch_one(
            "SELECT * FROM accounts WHERE account_id = ?",
            (account_id,)
        )
        if existing_account:
            return False

        # Create new account in database
        account = Account(account_id, account_name, user_id, initial_balance)
        success = self.db.execute(
            "INSERT INTO accounts (account_id, account_name, user_id, balance, created_at) VALUES (?, ?, ?, ?, ?)",
            (account.account_id, account.account_name, account.user_id, account.get_balance(), account.created_at.isoformat())
        )
        return success

    def get_account(self, account_id: str) -> Optional[Account]:
        """
        Args:
            account_id: Account ID to retrieve
        Returns:
            Account or None if not found
        """
        if not self.db:
            return None

        account_row = self.db.fetch_one(
            "SELECT * FROM accounts WHERE account_id = ?",
            (account_id,)
        )

        if account_row:
            account = Account(
                account_row['account_id'],
                account_row['account_name'],
                account_row['user_id'],
                account_row['balance'],
                datetime.fromisoformat(account_row['created_at'])
            )
            return account
        return None

    def get_user_accounts(self, user_id: str) -> List[Account]:
        """
        Args:
            user_id: get all accounts based on user id
        Returns:
            List of accounts owned by user
        """
        if not self.db:
            return []
        account_rows = self.db.fetch_all(
            "SELECT * FROM accounts WHERE user_id = ?",
            (user_id,)
        )

        accounts = []
        for row in account_rows:
            account = Account(
                row['account_id'],
                row['account_name'],
                row['user_id'],
                row['balance'],
                datetime.fromisoformat(row['created_at'])
            )
            accounts.append(account)

        return accounts

    def update_account(
        self,
        account_id: str,
        account_name: Optional[str] = None,
        balance: Optional[float] = None
    ) -> bool:
        """
        Args:
            account_id: Account ID to update
            account_name: New account name
            balance: New balance (if updating)      
        Returns:
            bool: True if update successful
        """
        if not self.db:
            return False

        account = self.get_account(account_id)
        if not account:
            return False

        updates = []
        params = []

        if account_name:
            updates.append("account_name = ?")
            params.append(account_name)

        if balance is not None:
            updates.append("balance = ?")
            params.append(balance)

        if not updates:
            return True

        params.append(account_id)
        query = "UPDATE accounts SET " + ", ".join(updates) + " WHERE account_id = ?"

        return self.db.execute(query, tuple(params))

    def delete_account(self, account_id: str, user_id: str) -> bool:
        """
        Args:
            account_id: Account ID to delete
            user_id: Owner user ID (for verification)
        Returns:
            bool: True if deletion successful, False if it's the user's only account
        """
        if not self.db:
            return False

        account = self.get_account(account_id)
        if not account or account.user_id != user_id:
            return False

        # Check if this is the user's only account
        user_accounts = self.get_user_accounts(user_id)
        if len(user_accounts) <= 1:
            # Cannot delete if it's the only account
            return False

        return self.db.execute(
            "DELETE FROM accounts WHERE account_id = ?",
            (account_id,)
        )
