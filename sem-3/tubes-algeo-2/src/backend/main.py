from contextlib import asynccontextmanager
import logging

from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import JSONResponse
from fastapi.staticfiles import StaticFiles

from app.api.core.config import get_settings
from app.api.core.dependencies import init_all
from app.api.routers.books import router as books_router
from app.api.routers.search import router as search_router
from app.api.routers.recommend import router as recommendations_router

settings = get_settings()

logger = logging.getLogger("uvicorn.error")

@asynccontextmanager
async def lifespan(app: FastAPI):
    """Initialize all processors on startup."""
    logger.info("Initializing all processors")
    init_all(settings)
    logger.info("Successfully initialized all processors")
    yield

app = FastAPI(title=settings.app_title, lifespan=lifespan)

# Routers
app.include_router(books_router, prefix="/books", tags=["Books"])
app.include_router(search_router, prefix="/search", tags=["Search"])
app.include_router(recommendations_router, prefix="/recommend", tags=["Recommendations"])

# Static files
app.mount("/covers", StaticFiles(directory=settings.covers_dir), name="covers")
app.mount("/txt", StaticFiles(directory=settings.txt_dir), name="txt")

# CORS middleware
app.add_middleware(
    CORSMiddleware,
    allow_origins=[settings.cors_origins],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)


@app.exception_handler(Exception)
async def global_exception_handler(request, exc):
    return JSONResponse(
        status_code=500,
        content={
            "error": "Internal Server Error",
            "message": str(exc),
        },
    )


@app.get("/")
async def root():
    return {"message": "Hello World"}


if __name__ == "__main__":
    import uvicorn

    uvicorn.run(
        "main:app",
        host="0.0.0.0",
        port=8000,
        reload=True,
        log_level="debug",
    )
