import json
import os

logger = __import__('logging').getLogger("uvicorn.error")

class TextProcessor:
    def __init__(self, mapper_file: str = ""):
        self.texts = []
        self.book_ids = []
        self.titles = []

        if mapper_file:
            self.load_mapper(mapper_file)

    def load_mapper(self, mapper_file: str):
        if not os.path.exists(mapper_file):
            raise FileNotFoundError(f"Mapper file {mapper_file} does not exist.")

        with open(mapper_file, "r", encoding="utf-8") as f:
            mapper = json.load(f)

        base_dir = os.path.dirname(mapper_file)

        for book_id, meta in mapper.items():
            txt_rel = meta.get("txt")
            if not txt_rel:
                logger.warning(f"Missing txt path for book {book_id}")
                continue

            txt_path = os.path.join(base_dir, txt_rel)

            if not os.path.exists(txt_path):
                logger.warning(f"Missing text file for book {book_id} -> {txt_path}")
                continue

            with open(txt_path, "r", encoding="utf-8") as f:
                content = f.read()

            self.book_ids.append(book_id)
            self.titles.append(meta.get("title", "Unknown Title"))
            self.texts.append(content)

    def get_all_texts(self):
        return self.texts

    def get_book_meta(self):
        return list(zip(self.book_ids, self.titles))
