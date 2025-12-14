import os
from pathlib import Path
from functools import lru_cache

class Settings:
    def __init__(self):
        self.backend_dir: Path = Path(__file__).resolve().parent.parent.parent.parent
        
        # Check if running in Docker
        docker_data_dir = Path("/app/data")
        if docker_data_dir.exists():
            self.data_dir: Path = docker_data_dir
            self.project_root: Path = Path("/app")
        else:
            self.project_root: Path = self.backend_dir.parent.parent
            self.data_dir: Path = self.project_root / "data"
        
        self.covers_dir: Path = self.data_dir / "covers"
        self.covers_grayscale_dir: Path = self.data_dir / "covers_grayscale"
        self.txt_dir: Path = self.data_dir / "txt"
        self.mapper_path: Path = self.data_dir / "mapper.json"
        self.cache_dir: Path = self.data_dir / "cache"
        
        self.lsa_k: int = 150
        
        self.app_title: str = "ITBooks"
        self.cors_origins: str = os.getenv("CORS_ORIGINS", "*")

@lru_cache
def get_settings() -> Settings:
    return Settings()
