import hashlib

class Security:
    @staticmethod  # don't use self
    def hash_password(password: str) -> str:
        """
        Returns:
            str: hashed password (hex string)
        """
        return hashlib.sha256(password.encode()).hexdigest()
