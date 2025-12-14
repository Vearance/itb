import { NextRequest, NextResponse } from "next/server";

const INTERNAL_API_URL = process.env.INTERNAL_API_URL || process.env.NEXT_PUBLIC_API_URL || "http://localhost:8000";
const PUBLIC_API_URL = process.env.NEXT_PUBLIC_API_URL || "http://localhost:8000";

interface Book {
  id: string;
  title: string;
  cover?: string;
  text?: string;
}

export async function GET(
  request: NextRequest,
  { params }: { params: Promise<{ id: string }> }
): Promise<NextResponse> {
  try {
    const { id } = await params;

    const response = await fetch(`${INTERNAL_API_URL}/books/${id}`, {
      cache: "no-store",
    });

    if (!response.ok) {
      return NextResponse.json(
        { success: false, error: "Book not found" },
        { status: 404 }
      );
    }

    const data = await response.json();

    let coverUrl = "";
    if (data.cover) {
      const filename = data.cover.split("/").pop();
      coverUrl = `${PUBLIC_API_URL}/covers/${filename}`;
    }

    const book: Book = {
      id: data.id,
      title: data.title,
      cover: coverUrl,
      text: data.text,
    };

    return NextResponse.json({
      success: true,
      data: book,
    });
  } catch (error) {
    console.error("Error in GET /api/books/[id]:", error);
    return NextResponse.json(
      {
        success: false,
        error:
          error instanceof Error
            ? error.message
            : "Failed to fetch book",
      },
      { status: 500 }
    );
  }
}
