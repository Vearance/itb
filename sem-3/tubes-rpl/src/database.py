import sqlite3
from typing import Optional

class Database:
    def __init__(self, db_path: str = "tabungin.db"):
        """
        Args:
            db_path: path to SQLite database file
        """
        self.db_path = db_path
        self.connection: Optional[sqlite3.Connection] = None

    def connect(self) -> bool:
        """
        Returns:
            bool: True if successful
        """
        try:
            self.connection = sqlite3.connect(self.db_path)
            self.connection.row_factory = sqlite3.Row
            return True
        except sqlite3.Error as e:
            print(f"Database connection error: {e}")
            return False

    def disconnect(self) -> bool:
        """
        Returns:
            bool: True if disconnection successful
        """
        try:
            if self.connection:
                self.connection.close()
                return True
            return False
        except sqlite3.Error as e:
            print(f"Database disconnection error: {e}")
            return False

    def execute(self, query: str, params: tuple = ()) -> bool:
        """
        exec a query
        Args:
            query: SQL query string
            params: Query parameters
        Returns:
            bool: True if exec successful
        """
        try:
            cursor = self.connection.cursor()
            cursor.execute(query, params)
            self.connection.commit()
            return True
        except sqlite3.Error as e:
            print(f"Query execution error: {e}")
            return False

    def fetch_one(self, query: str, params: tuple = ()):
        """
        Fetch one row from database
        Args:
            query: SQL query string
            params: Query parameters
        Returns:
            Row data or None if not found
        """
        try:
            cursor = self.connection.cursor()
            cursor.execute(query, params)
            return cursor.fetchone()
        except sqlite3.Error as e:
            print(f"Query fetch error: {e}")
            return None

    def fetch_all(self, query: str, params: tuple = ()):
        """
        Fetch all rows from database
        Args:
            query: SQL query string
            params: Query parameters   
        Returns:
            List of rows or empty list
        """
        try:
            cursor = self.connection.cursor()
            cursor.execute(query, params)
            return cursor.fetchall()
        except sqlite3.Error as e:
            print(f"Query fetch error: {e}")
            return []

    def init_tables(self) -> bool:
        """
        Initialize database tables
        Returns:
            bool: True if initialization successful
        """
        try:
            cursor = self.connection.cursor()

            # Users table
            cursor.execute('''
                CREATE TABLE IF NOT EXISTS users (
                    user_id TEXT PRIMARY KEY,
                    username TEXT UNIQUE NOT NULL,
                    password TEXT NOT NULL
                )
            ''')

            # Accounts table
            cursor.execute('''
                CREATE TABLE IF NOT EXISTS accounts (
                    account_id TEXT PRIMARY KEY,
                    account_name TEXT NOT NULL,
                    user_id TEXT NOT NULL,
                    balance REAL DEFAULT 0.0,
                    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                    FOREIGN KEY (user_id) REFERENCES users (user_id)
                )
            ''')

            # Transactions table
            cursor.execute('''
                CREATE TABLE IF NOT EXISTS transactions (
                    transaction_id TEXT PRIMARY KEY,
                    account_id TEXT NOT NULL,
                    amount REAL NOT NULL,
                    type TEXT NOT NULL,
                    description TEXT,
                    date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                    FOREIGN KEY (account_id) REFERENCES accounts (account_id)
                )
            ''')

            # Targets table
            cursor.execute('''
                CREATE TABLE IF NOT EXISTS targets (
                    target_id TEXT PRIMARY KEY,
                    account_id TEXT NOT NULL,
                    target_name TEXT NOT NULL,
                    target_amount REAL NOT NULL,
                    current_amount REAL DEFAULT 0.0,
                    is_achieved INTEGER DEFAULT 0,
                    is_archived INTEGER DEFAULT 0,
                    deadline TIMESTAMP NOT NULL,
                    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                    FOREIGN KEY (account_id) REFERENCES accounts (account_id)
                )
            ''')

            self.connection.commit()
            return True
        except sqlite3.Error as e:
            print(f"Table initialization error: {e}")
            return False
