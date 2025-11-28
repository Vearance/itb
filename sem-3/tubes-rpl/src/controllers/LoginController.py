from typing import Optional
from models.User import User
from database import Database
from utility.Security import Security


class LoginController:
    def __init__(self, db: Database = None):
        self.current_user: Optional[User] = None
        self.db = db

    def register_user(self, user_id: str, username: str, password: str) -> bool:
        """
        Returns:
            bool: True if registration successful, False if user exists
        """
        if not self.db:
            return False

        # Check if user already exists (by username)
        existing_user = self.db.fetch_one(
            "SELECT * FROM users WHERE username = ?",
            (username,)
        )
        if existing_user:
            return False

        # Create new user and insert into database
        user = User(user_id, username, password)
        success = self.db.execute(
            "INSERT INTO users (user_id, username, password) VALUES (?, ?, ?)",
            (user.user_id, user.username, user.password)
        )
        return success

    def login(self, username: str, password: str) -> str:
        """
        Returns:
            str: "SUCCESS", "USER_NOT_FOUND", or "WRONG_PASSWORD"
        """
        if not self.db:
            return "DB_ERROR"

        # Query database for user by username
        user_row = self.db.fetch_one(
            "SELECT * FROM users WHERE username = ?",
            (username,)
        )

        if not user_row:
            return "USER_NOT_FOUND"

        # Hash the input password
        hashed_input = Security.hash_password(password)

        # Compare hashes directly
        if user_row['password'] == hashed_input:
            # Create User object with hashed password (don't hash again)
            user = User(user_row['user_id'], user_row['username'], password)
            # Override the hashed password with the one from DB
            user.password = user_row['password']
            self.current_user = user
            return "SUCCESS"
            
        return "WRONG_PASSWORD"

    def logout(self) -> bool:
        """
        Returns:
            bool: True if logout successful
        """
        if self.current_user:
            self.current_user = None
            return True
        return False

    def is_logged_in(self) -> bool:
        if self.current_user:
            return True
        return False

    def get_current_user(self) -> Optional[User]:
        """Get currently logged in user"""
        return self.current_user

    def update_password(self, user_id: str, old_password: str, new_password: str) -> bool:
        """
        Args:
            user_id: which user to update
            old_password: current password for verif
            new_password: new password to set
        Returns:
            bool: True if update successful, False if old password incorrect or user not found
        """
        if not self.db:
            return False

        user_row = self.db.fetch_one(
            "SELECT * FROM users WHERE user_id = ?",
            (user_id,)
        )

        if not user_row:
            return False

        # Verify old password
        if user_row['password'] != Security.hash_password(old_password):
            return False

        # Hash new password and update in database
        new_hashed_password = Security.hash_password(new_password)
        return self.db.execute(
            "UPDATE users SET password = ? WHERE user_id = ?",
            (new_hashed_password, user_id)
        )
