# RekeningEntity class
from datetime import datetime
from typing import Optional

class Account:
    """
    Attributes:
        account_id: identifier; unique
        account_name: name of the account
        user_id: creator/owner's user_id
        initial_balance: starting balance (from 0)
        created_at: datetime of account creation
    """

    def __init__(
        self,
        account_id: str,
        account_name: str,
        user_id: str,
        initial_balance: float = 0.0,
        created_at: Optional[datetime] = None
    ):
        self.account_id = account_id
        self.account_name = account_name
        self.user_id = user_id
        self.balance = initial_balance
        self.created_at = created_at or datetime.now()

    def get_balance(self) -> float:
        return self.balance

    def set_balance(self, amount: float) -> bool:
        """
        Args:
            amount: new balance amount
        Returns:
            bool: True if successful
        """
        if amount < 0:
            return False
        self.balance = amount
        return True

    def add_balance(self, amount: float) -> bool:
        """
        Args:
            amount: amount to add
        Returns:
            bool: True if successful
        """
        if amount < 0:
            return False
        self.balance += amount
        return True

    def subtract_balance(self, amount: float) -> bool:
        """
        Args:
            amount: amount to subtract
        Returns:
            bool: True if successful, False if balance is insufficient
        """
        if amount < 0 or amount > self.balance:
            return False
        self.balance -= amount
        return True

    # TODO: implement if needed
    def to_dict(self) -> dict:
        """Convert account object to dictionary"""
        return {
            'account_id': self.account_id,
            'account_name': self.account_name,
            'user_id': self.user_id,
            'balance': self.balance,
            'created_at': self.created_at.isoformat()
        }

    def __repr__(self) -> str:
        return f"Account(id={self.account_id}, name={self.account_name}, balance={self.balance})"
