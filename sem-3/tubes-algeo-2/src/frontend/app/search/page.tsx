'use client';

import { BookCard } from "@/components/book-card";
import { useSearchParams } from "next/navigation";
import { useEffect, useState, Suspense } from "react";

interface Book {
  id: string;
  title: string;
  cover?: string;
}

function SearchContent() {
  const searchParams = useSearchParams();
  const query = searchParams.get("q") || "";
  const [results, setResults] = useState<Book[]>([]);
  const [isLoading, setIsLoading] = useState(false);

  useEffect(() => {
    const performSearch = async () => {
      if (!query.trim()) {
        setResults([]);
        return;
      }

      setIsLoading(true);
      try {
        const APP_URL = process.env.NEXT_PUBLIC_APP_URL || "http://localhost:3000";
        const params = new URLSearchParams({ q: query });
        const res = await fetch(`${APP_URL}/api/search?${params}`);

        if (!res.ok) {
          throw new Error("Failed to search books");
        }

        const response = await res.json();

        if (response.success) {
          setResults(response.data);
        }
      } catch (error) {
        console.error("Error searching books:", error);
        setResults([]);
      } finally {
        setIsLoading(false);
      }
    };

    performSearch();
  }, [query]);

  return (
    <main className="min-h-screen w-full px-3 sm:px-4 py-4 sm:py-6 lg:py-8 flex justify-center bg-background">
      <div className="w-full max-w-6xl">
        {query ? (
          <>
            <h1 className="text-lg sm:text-xl lg:text-2xl font-bold mb-1 sm:mb-2 text-foreground">
              Hasil Pencarian untuk "{query}"
            </h1>
            <p className="text-sm sm:text-base text-muted-foreground mb-4 sm:mb-6 lg:mb-8">
              Ditemukan {results.length} buku
            </p>

            {isLoading ? (
              <div className="flex items-center justify-center py-12 sm:py-16">
                <div className="text-center">
                  <div className="inline-block animate-spin rounded-full h-8 w-8 sm:h-10 sm:w-10 border-2 border-primary border-t-transparent mb-3 sm:mb-4"></div>
                  <p className="text-muted-foreground text-base sm:text-lg">Mencari...</p>
                </div>
              </div>
            ) : results.length > 0 ? (
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
                <p className="text-muted-foreground text-base sm:text-lg text-center px-4">
                  Tidak ada buku yang sesuai dengan pencarian Anda
                </p>
              </div>
            )}
          </>
        ) : (
          <div className="flex items-center justify-center py-12 sm:py-16">
            <p className="text-muted-foreground text-base sm:text-lg text-center px-4">
              Gunakan search bar untuk mencari buku
            </p>
          </div>
        )}
      </div>
    </main>
  );
}

function SearchLoading() {
  return (
    <main className="min-h-screen w-full px-3 sm:px-4 py-4 sm:py-6 lg:py-8 flex justify-center bg-background">
      <div className="w-full max-w-6xl">
        <div className="flex items-center justify-center py-12 sm:py-16">
          <div className="text-center">
            <div className="inline-block animate-spin rounded-full h-8 w-8 sm:h-10 sm:w-10 border-2 border-primary border-t-transparent mb-3 sm:mb-4"></div>
            <p className="text-muted-foreground text-base sm:text-lg">Memuat...</p>
          </div>
        </div>
      </div>
    </main>
  );
}

export default function SearchPage() {
  return (
    <Suspense fallback={<SearchLoading />}>
      <SearchContent />
    </Suspense>
  );
}
