from fastapi import APIRouter, HTTPException
from app.api.core.dependencies import BookRepositoryDep

router = APIRouter()

@router.get("/all")
def get_all_books(repo: BookRepositoryDep):
    books = repo.get_all()
    return books

@router.get("/{book_id}")
def get_book(book_id: str, repo: BookRepositoryDep):
    book = repo.get(book_id)
    if not book:
        raise HTTPException(404, "Book not found")

    return {
        "id": book_id,
        "title": book.get("title"),
        "cover": book.get("cover"),
        "text": book.get("txt"),
    }
