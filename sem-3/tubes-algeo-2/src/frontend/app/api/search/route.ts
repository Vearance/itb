import { NextRequest, NextResponse } from "next/server";

const INTERNAL_API_URL = process.env.INTERNAL_API_URL || process.env.NEXT_PUBLIC_API_URL || "http://localhost:8000";
const PUBLIC_API_URL = process.env.NEXT_PUBLIC_API_URL || "http://localhost:8000";

interface Book {
  id: string;
  title: string;
  cover?: string;
}

export async function GET(request: NextRequest): Promise<NextResponse> {
  try {
    const searchParams = request.nextUrl.searchParams;
    const query = searchParams.get("q") || "";

    if (!query.trim()) {
      return NextResponse.json({
        success: true,
        data: [],
      });
    }

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
        coverUrl = `${PUBLIC_API_URL}/covers/${filename}`;
      }

      return {
        id,
        title: book.title || `Book ${id}`,
        cover: coverUrl,
      };
    }).filter((book) =>
      book.title.toLowerCase().includes(query.toLowerCase())
    );

    return NextResponse.json({
      success: true,
      data: books,
    });
  } catch (error) {
    console.error("Error in GET /api/search:", error);
    return NextResponse.json(
      {
        success: false,
        error:
          error instanceof Error
            ? error.message
            : "Failed to search books",
      },
      { status: 500 }
    );
  }
}
