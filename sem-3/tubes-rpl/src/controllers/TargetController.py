from typing import List, Optional
from datetime import datetime
from models.Target import Target
from database import Database


class TargetController:
    def __init__(self, db: Database = None):
        """Initialize TargetController"""
        self.db = db

    def create_target(
        self,
        target_id: str,
        account_id: str,
        target_name: str,
        target_amount: float,
        deadline: datetime
    ) -> bool:
        """
        Returns:
            bool: True if target was created
        """
        if not self.db:
            return False

        # Check if target already exists
        existing = self.db.fetch_one(
            "SELECT * FROM targets WHERE target_id = ?",
            (target_id,)
        )
        if existing:
            return False

        target = Target(target_id, account_id, target_name, target_amount, deadline)

        success = self.db.execute(
            "INSERT INTO targets (target_id, account_id, target_name, target_amount, current_amount, is_achieved, is_archived, deadline, created_at) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)",
            (target.target_id, target.account_id, target.target_name, target.target_amount, target.current_amount, target.is_achieved, target.is_archived, target.deadline.isoformat(), target.created_at.isoformat())
        )

        return success

    def get_target(self, target_id: str) -> Optional[Target]:
        """
        Args:
            target_id: to get by id
        Returns:
            Target or None if not found
        """
        if not self.db:
            return None

        target_row = self.db.fetch_one(
            "SELECT * FROM targets WHERE target_id = ?",
            (target_id,)
        )

        if target_row:
            target = Target(
                target_row['target_id'],
                target_row['account_id'],
                target_row['target_name'],
                target_row['target_amount'],
                datetime.fromisoformat(target_row['deadline']),
                target_row['current_amount'],
                datetime.fromisoformat(target_row['created_at'])
            )
            target.is_achieved = bool(target_row['is_achieved'])
            target.is_archived = bool(target_row['is_archived'])
            return target

        return None

    def get_account_targets(self, account_id: str) -> List[Target]:
        """
        Args:
            account_id: to get all targets by account id
        Returns:
            List of targets for the account
        """
        if not self.db:
            return []

        target_rows = self.db.fetch_all(
            "SELECT * FROM targets WHERE account_id = ?",
            (account_id,)
        )

        targets = []
        for row in target_rows:
            target = Target(
                row['target_id'],
                row['account_id'],
                row['target_name'],
                row['target_amount'],
                datetime.fromisoformat(row['deadline']),
                row['current_amount'],
                datetime.fromisoformat(row['created_at'])
            )
            target.is_achieved = bool(row['is_achieved'])
            target.is_archived = bool(row['is_archived'])
            targets.append(target)

        return targets

    def update_target(
        self,
        target_id: str,
        target_name: Optional[str] = None,
        target_amount: Optional[float] = None,
        deadline: Optional[datetime] = None
    ) -> bool:
        """
        Update target properties
        Args:
            target_id: Target ID to update
            target_name: New target name
            target_amount: New target amount
            deadline: New deadline
        Returns:
            bool: True if update successful
        """
        if not self.db:
            return False

        target = self.get_target(target_id)
        if not target:
            return False

        updates = []
        params = []

        if target_name:
            updates.append("target_name = ?")
            params.append(target_name)

        if target_amount is not None:
            updates.append("target_amount = ?")
            params.append(target_amount)

        if deadline:
            updates.append("deadline = ?")
            params.append(deadline.isoformat())

        if not updates:
            return True

        params.append(target_id)
        query = "UPDATE targets SET " + ", ".join(updates) + " WHERE target_id = ?"

        return self.db.execute(query, tuple(params))

    def add_progress(self, target_id: str, amount: float) -> bool:
        """
        Args:
            target_id: Target ID
            amount: Amount to add
        Returns:
            bool: True if progress added successfully
        """
        if not self.db:
            return False

        target = self.get_target(target_id)
        if not target:
            return False

        target.add_amount(amount)

        # Update database with current_amount, is_achieved, and is_archived
        return self.db.execute(
            "UPDATE targets SET current_amount = ?, is_achieved = ?, is_archived = ? WHERE target_id = ?",
            (target.current_amount, target.is_achieved, target.is_archived, target_id)
        )

    def delete_target(self, target_id: str, account_id: str) -> bool:
        """
        Args:
            target_id: Target ID to delete
            account_id: Account ID
        Returns:
            bool: True if deletion successful
        """
        if not self.db:
            return False

        target = self.get_target(target_id)
        if not target or target.account_id != account_id:
            return False

        return self.db.execute(
            "DELETE FROM targets WHERE target_id = ?",
            (target_id,)
        )

    def get_target_progress(self, target_id: str) -> Optional[float]:
        """
        Args:
            target_id: Target ID   
        Returns:
            Progress percentage or None if target not found
        """
        target = self.get_target(target_id)
        if not target:
            return None

        return target.get_percentage()

    def is_target_achieved(self, target_id: str) -> Optional[bool]:
        """
        Args:
            target_id: Target ID  
        Returns:
            True if achieved, False if not, None if target not found
        """
        target = self.get_target(target_id)
        if not target:
            return None
        return target.is_achieved
