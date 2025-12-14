from fastapi import APIRouter, Query, HTTPException

from app.api.core.dependencies import DocProcessorDep

router = APIRouter()


@router.get("")
def get_recommendations(
    lsa: DocProcessorDep,
    book_id: str = Query(..., description="Book ID to get recommendations for"),
    top_k: int = Query(5, description="Number of results"),
):
    """
    Get book recommendations based on a book ID.

    Returns top-k similar books based on content similarity (LSA).
    """
    # Check if book exists first
    if not any(str(bid) == str(book_id) for bid, _ in lsa.book_meta):
        raise HTTPException(status_code=404, detail=f"Book ID {book_id} not found")

    results = lsa.recommend_by_id(book_id=book_id, top_k=top_k)

    return {
        "book_id": book_id,
        "results": results,
    }
