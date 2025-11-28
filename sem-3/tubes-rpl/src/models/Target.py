from datetime import datetime
from typing import Optional


class Target:
    """
    Attributes:
        target_id: identifier; unique
        account_id: identifier of associated account
        target_name: name of the target
        target_amount: target goal amount
        current_amount: current amount saved towards target
        deadline: target deadline
        created_at: target creation timestamp
    """

    def __init__(
        self,
        target_id: str,
        account_id: str,
        target_name: str,
        target_amount: float,
        deadline: datetime,
        current_amount: float = 0.0,
        created_at: Optional[datetime] = None
    ):
        self.target_id = target_id
        self.account_id = account_id
        self.target_name = target_name
        self.target_amount = target_amount
        self.current_amount = current_amount
        self.deadline = deadline
        self.created_at = created_at or datetime.now()
        self.is_achieved = self.calculate_achieved()
        self.is_archived = False  # Track if target is archived

    def add_amount(self, amount: float) -> bool:
        """
        Returns:
            bool: True if successful, False if invalid amount
        """
        if amount < 0 or self.is_archived:
            return False
        self.current_amount += amount
        self.is_achieved = self.calculate_achieved()

        # Reset progress ke 0 saat target tercapai, jangan auto-archive
        if self.is_achieved:
            self.current_amount = 0.0
            self.is_achieved = False

        return True

    def subtract_amount(self, amount: float) -> bool:
        """
        Returns:
            bool: True if successful, False if insufficient or archived
        """
        # Cannot withdraw from archived target
        if self.is_archived:
            return False

        if amount < 0 or amount > self.current_amount:
            return False
        self.current_amount -= amount
        self.is_achieved = self.calculate_achieved()
        return True

    def calculate_achieved(self) -> bool:
        """Calculate if target is achieved"""
        return self.current_amount >= self.target_amount

    def get_percentage(self) -> float:
        """ 
        Returns:
            float: Progress percentage (0-100)
        """
        if self.target_amount == 0:
            return 0.0
        percentage = (self.current_amount / self.target_amount) * 100
        return min(percentage, 100.0)

    def get_remaining_amount(self) -> float:
        """
        Returns:
            float: Remaining amount (0 if already achieved)
        """
        if self.is_achieved:
            return 0.0
        else:
            return self.target_amount - self.current_amount

    def to_dict(self) -> dict:
        """Convert target object to dictionary"""
        return {
            'target_id': self.target_id,
            'account_id': self.account_id,
            'target_name': self.target_name,
            'target_amount': self.target_amount,
            'current_amount': self.current_amount,
            'percentage': self.get_percentage(),
            'is_achieved': self.is_achieved,
            'is_archived': self.is_archived,
            'deadline': self.deadline.isoformat(),
            'created_at': self.created_at.isoformat()
        }

    def __repr__(self) -> str:
        return (f"Target(id={self.target_id}, "
                f"name={self.target_name}, "
                f"progress={self.get_percentage():.1f}%)")
