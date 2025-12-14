import { NextRequest, NextResponse } from "next/server";

const INTERNAL_API_URL = process.env.INTERNAL_API_URL || process.env.NEXT_PUBLIC_API_URL || "http://localhost:8000";
const PUBLIC_API_URL = process.env.NEXT_PUBLIC_API_URL || "http://localhost:8000";

interface Book {
  id: string;
  title: string;
  cover?: string;
  txt?: string;
}

export async function GET(request: NextRequest): Promise<NextResponse> {
  try {
    const searchParams = request.nextUrl.searchParams;
    const page = parseInt(searchParams.get("page") || "1", 10);
    const limit = parseInt(searchParams.get("limit") || "10", 10);

    const response = await fetch(`${INTERNAL_API_URL}/books/all`, {
      cache: "no-store",
    });

    if (!response.ok) {
      throw new Error("Failed to fetch books from backend");
    }

    const data = await response.json();
    const books: Book[] = Object.entries(data).map(([id, book]: [string, any]) => {
      let coverUrl = "";
      if (book.cover) {
        const filename = book.cover.split("/").pop();
        // Use public URL for cover images (accessed by browser)
        coverUrl = `${PUBLIC_API_URL}/covers/${filename}`;
      }

      return {
        id,
        title: book.title || `Book ${id}`,
        cover: coverUrl,
        txt: book.txt || "",
      };
    });

    // pagination
    const totalBooks = books.length;
    const totalPages = Math.ceil(totalBooks / limit);
    const validPage = Math.max(1, Math.min(page, totalPages || 1));
    const startIndex = (validPage - 1) * limit;
    const endIndex = startIndex + limit;
    const paginatedBooks = books.slice(startIndex, endIndex);

    return NextResponse.json({
      success: true,
      data: paginatedBooks,
      pagination: {
        currentPage: validPage,
        totalPages,
        totalBooks,
        limit,
      },
    });
  } catch (error) {
    console.error("Error in GET /api/books:", error);
    return NextResponse.json(
      {
        success: false,
        error:
          error instanceof Error
            ? error.message
            : "Failed to fetch books",
      },
      { status: 500 }
    );
  }
}
