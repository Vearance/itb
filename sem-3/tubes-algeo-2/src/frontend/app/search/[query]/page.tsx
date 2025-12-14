import { BookCard } from "@/components/book-card";

interface Book {
  id: string;
  title: string;
  cover?: string;
}

async function searchBooks(query: string): Promise<Book[]> {
  if (!query.trim()) return [];

  try {
    const APP_URL = process.env.NEXT_PUBLIC_APP_URL || "http://localhost:3000";
    const params = new URLSearchParams({ q: query });
    const res = await fetch(`${APP_URL}/api/search?${params}`, {
      cache: "no-store",
    });

    if (!res.ok) {
      throw new Error("Failed to search books");
    }

    const response = await res.json();

    if (!response.success) {
      return [];
    }

    return response.data;
  } catch (error) {
    console.error("Error searching books:", error);
    return [];
  }
}

export default async function SearchResults({
  params,
}: {
  params: Promise<{ query: string }>;
}) {
  const { query } = await params;
  const decodedQuery = decodeURIComponent(query);

  const results = await searchBooks(decodedQuery);

  return (
    <main className="w-full px-3 sm:px-4 py-4 sm:py-6 lg:py-8 flex justify-center">
      <div className="w-full max-w-6xl">
        <h1 className="text-lg sm:text-xl lg:text-2xl font-bold mb-1 sm:mb-2">
          Hasil Pencarian untuk "{decodedQuery}"
        </h1>
        <p className="text-sm sm:text-base text-gray-600 mb-4 sm:mb-6 lg:mb-8">
          Ditemukan {results.length} buku
        </p>

        {results.length > 0 ? (
          <div className="grid grid-cols-2 sm:grid-cols-3 md:grid-cols-4 lg:grid-cols-5 gap-2 sm:gap-3 lg:gap-4">
            {results.map((book) => (
              <BookCard
                key={book.id}
                id={book.id}
                title={book.title}
                cover={book.cover}
                txt=""
              />
            ))}
          </div>
        ) : (
          <div className="flex items-center justify-center py-12 sm:py-16">
            <p className="text-gray-600 text-base sm:text-lg text-center px-4">Tidak ada buku yang sesuai dengan pencarian Anda</p>
          </div>
        )}
      </div>
    </main>
  );
}
