from fastapi import APIRouter, Query, UploadFile, File, HTTPException
import filetype
import PIL.Image as Image
from app.api.core.dependencies import DocProcessorDep, ImageProcessorDep
from app.api.core.config import get_settings

router = APIRouter()


@router.get("")
def search_by_text(
    lsa: DocProcessorDep,
    q: str = Query(..., description="Search query"),
    top_k: int = Query(5, description="Number of results"),
):
    results = lsa.search(query=q, top_k=top_k, k=get_settings().lsa_k)
    return {
        "query": q,
        "results": results,
    }


@router.post("/document")
async def search_by_document(
    lsa: DocProcessorDep,
    file: UploadFile = File(..., description="Text file to search with"),
    top_k: int = Query(5, description="Number of results"),
):
    if not file.filename or not file.filename.endswith(".txt"):
        raise HTTPException(
            status_code=400, detail="Unsupported file format. Only .txt files are supported")

    content = await file.read()
    query_text = content.decode("utf-8")
    settings = get_settings()

    results = lsa.search(query=query_text, top_k=top_k, k=settings.lsa_k)
    return {
        "success": True,
        "data": results,
    }


@router.post("/image")
async def search_by_image(
    image_processor: ImageProcessorDep,
    file: UploadFile = File(..., description="Image file to search with"),
    top_k: int = Query(5, description="Number of results"),
):
    import tempfile
    import os

    with tempfile.NamedTemporaryFile(delete=False, suffix=os.path.splitext(file.filename or "")[1]) as tmp:
        content = await file.read()
        tmp.write(content)
        tmp_path = tmp.name

    if filetype.is_image(tmp_path) is None:
        os.unlink(tmp_path)
        raise HTTPException(
            status_code=400, detail="Unsupported file format. Please upload a valid image file.")

    with Image.open(tmp_path) as img:
        try:
            if img.size != (200, 300):
                img.resize((200, 300)).save(tmp_path)
        except Exception:
            os.unlink(tmp_path)
            raise HTTPException(
                status_code=400, detail="Error processing image. Please upload a valid image file.")      

    try:
        results = image_processor.search_cover_by_image(
            query_image_path=tmp_path, top_k=top_k)
    finally:
        os.unlink(tmp_path)

    return {
        "success": True,
        "data": results,
    }
