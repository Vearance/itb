import json
from pathlib import Path

class BookRepository:
    def __init__(self, mapper_path=None):
        if mapper_path is None:
            current_file = Path(__file__).resolve()
            project_root = current_file.parent.parent.parent.parent.parent
            mapper_path = project_root / "data" / "mapper.json"
        with open(mapper_path, "r", encoding="utf-8") as f:
            self.mapper = json.load(f)

    def get(self, book_id: str):
        return self.mapper.get(book_id)

    def get_all(self):
        return self.mapper
