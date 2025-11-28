# UserEntity class
from utility.Security import Security
# TODO: if error, move hash_password to this class
class User:
    """
    Attributes:
        user_id: identifier; unique
        username: username of user
        password: (hashed)
    """
    def __init__(
        self,
        user_id: str,
        username: str,
        password: str
    ):
        self.user_id = user_id
        self.username = username
        self.password = Security.hash_password(password)

    def verify_password(self, password: str) -> bool:
        return self.password == Security.hash_password(password)

    # TODO: implement if needed
    def to_dict(self) -> dict:
        """Convert user object to dictionary"""
        return {
            'user_id': self.user_id,
            'username': self.username
        }

    def __repr__(self) -> str:
        return f"User(id={self.user_id}, username={self.username})"
