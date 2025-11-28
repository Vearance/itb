from datetime import datetime
from enum import Enum
from typing import Optional


class TransactionType(Enum):
    INCOME = "income"
    EXPENSE = "expense"


class Transaction:
    """
    Attributes:
        transaction_id: identifier; unique
        account_id: account's transaction belongs to
        amount: amount of the transaction
        transaction_type: type of transaction; income or expense
        description: transaction description
        date: transaction datetime
    """

    def __init__(
        self,
        transaction_id: str,
        account_id: str,
        amount: float,
        transaction_type: TransactionType,
        description: str,
        date: Optional[datetime] = None
    ):
        self.transaction_id = transaction_id
        self.account_id = account_id
        self.amount = amount
        self.transaction_type = transaction_type
        self.description = description
        self.date = date or datetime.now()

    def is_income(self) -> bool:
        return self.transaction_type == TransactionType.INCOME

    def is_expense(self) -> bool:
        return self.transaction_type == TransactionType.EXPENSE

    def get_signed_amount(self) -> float:
        """
        Returns:
            float: if income, returns amount; if expense, returns negative amount
        """
        return self.amount if self.is_income() else -self.amount

    def to_dict(self) -> dict:
        """Convert transaction object to dictionary"""
        return {
            'transaction_id': self.transaction_id,
            'account_id': self.account_id,
            'amount': self.amount,
            'type': self.transaction_type.value,
            'description': self.description,
            'date': self.date.isoformat()
        }

    def __repr__(self) -> str:
        return (f"Transaction(id={self.transaction_id}, "
                f"type={self.transaction_type.value}, "
                f"amount={self.amount}, "
                f"desc={self.description})")
