from typing import Annotated, Optional
import os
from fastapi import Depends
from app.api.core.config import Settings, get_settings
from app.lib.text.lsa import LSA_Model, load_lsa_from_cache, build_and_cache_lsa
from app.lib.image.main import ImageProcessor
from app.lib.books import BookRepository

_lsa_model: Optional[LSA_Model] = None
_image_processor: Optional[ImageProcessor] = None
_book_repository: Optional[BookRepository] = None


def init_document_processor(settings: Settings) -> LSA_Model:
    global _lsa_model

    required_files = ["vocab.json", "tf_list.json", "idf.json",
                      "book_meta.json", "tfidf_matrix.npz", "u_k.npz", "s_k.npy", "doc_vectors.npy"]

    # Check if all required files exist
    cache_exists = all(os.path.exists(os.path.join(
        str(settings.cache_dir), f)) for f in required_files)

    if cache_exists:
        _lsa_model = load_lsa_from_cache(
            cache_dir=str(settings.cache_dir),
            k=settings.lsa_k
        )
    else:
        # Ensure data directory exists
        os.makedirs(str(settings.cache_dir), exist_ok=True)

        _lsa_model = build_and_cache_lsa(
            cache_dir=str(settings.cache_dir),
            k=settings.lsa_k,
            mapper_path=str(settings.mapper_path)
        )

    return _lsa_model


def init_image_processor(settings: Settings) -> ImageProcessor:
    global _image_processor

    _image_processor = ImageProcessor(
        cover_folder=str(settings.covers_dir),
        mapper_file=str(settings.mapper_path),
        cover_output_folder=str(settings.covers_grayscale_dir),
        cache_dir=str(settings.cache_dir)
    )
    return _image_processor


def init_book_repository(settings: Settings) -> BookRepository:
    global _book_repository
    _book_repository = BookRepository(mapper_path=settings.mapper_path)
    return _book_repository


def init_all(settings: Optional[Settings] = None) -> None:
    if settings is None:
        settings = get_settings()

    init_book_repository(settings)
    init_image_processor(settings)
    init_document_processor(settings)


def get_document_processor() -> LSA_Model:
    if _lsa_model is None:
        raise RuntimeError(
            "Document processor not initialized. Call init_document_processor() first.")
    return _lsa_model


def get_image_processor() -> ImageProcessor:
    global _image_processor
    if _image_processor is None:
        settings = get_settings()
        init_image_processor(settings)
    assert _image_processor is not None
    return _image_processor


def get_book_repository() -> BookRepository:
    if _book_repository is None:
        raise RuntimeError(
            "Book Repository not initialized. Call init_book_repository() first.")
    return _book_repository


DocProcessorDep = Annotated[LSA_Model, Depends(get_document_processor)]
ImageProcessorDep = Annotated[ImageProcessor, Depends(get_image_processor)]
BookRepositoryDep = Annotated[BookRepository, Depends(get_book_repository)]
SettingsDep = Annotated[Settings, Depends(get_settings)]
