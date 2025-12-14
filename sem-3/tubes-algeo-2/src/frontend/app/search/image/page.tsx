'use client';

import { BookCard } from "@/components/book-card";
import { useEffect, useState, useCallback } from "react";
import Image from "next/image";
import { performImageSearch } from "@/components/search/image-search";

interface Book {
  id: string;
  title: string;
  cover?: string;
  score?: number;
}

export default function ImageSearchPage() {
  const [results, setResults] = useState<Book[]>([]);
  const [isLoading, setIsLoading] = useState(false);
  const [uploadedImage, setUploadedImage] = useState<string | null>(null);

  const handleImageSearch = useCallback(async (imageBase64: string) => {
    setIsLoading(true);
    try {
      const books = await performImageSearch(imageBase64);
      setResults(books);
    } finally {
      setIsLoading(false);
    }
  }, []);

  useEffect(() => {
    const storedImage = sessionStorage.getItem("uploadedImage");
    if (storedImage) {
      setUploadedImage(storedImage);
      handleImageSearch(storedImage);
      sessionStorage.removeItem("uploadedImage");
    }
  }, [handleImageSearch]);

  useEffect(() => {
    const handleNewUpload = (event: CustomEvent<string>) => {
      const imageData = event.detail;
      setUploadedImage(imageData);
      handleImageSearch(imageData);
      sessionStorage.removeItem("uploadedImage");
    };

    window.addEventListener("newImageUpload", handleNewUpload as EventListener);
    return () => {
      window.removeEventListener("newImageUpload", handleNewUpload as EventListener);
    };
  }, [handleImageSearch]);

  if (!uploadedImage) {
    return (
      <main className="min-h-screen w-full px-3 sm:px-4 py-4 sm:py-8 flex justify-center bg-background">
        <div className="w-full max-w-7xl flex items-center justify-center py-12 sm:py-24">
          <p className="text-muted-foreground text-base sm:text-lg text-center px-4">
            Gunakan upload image untuk mencari buku berdasarkan cover buku.
          </p>
        </div>
      </main>
    );
  }

  return (
    <main className="min-h-screen w-full bg-background">
      <div className="w-full px-3 sm:px-4 py-4 sm:py-8 flex justify-center">
        <div className="w-full max-w-3xl">
          <div className="rounded-xl sm:rounded-2xl overflow-hidden border border-border shadow-lg shadow-primary/5 bg-card">
            <div className="relative w-full aspect-video">
              <Image
                src={uploadedImage}
                alt="Uploaded book cover"
                fill
                className="object-contain"
                priority
              />
            </div>
          </div>
        </div>
      </div>

      <div className="w-full px-3 sm:px-4 py-6 sm:py-12 flex justify-center">
        <div className="w-full max-w-6xl">
          <h2 className="text-xl sm:text-2xl font-bold mb-1 sm:mb-2 text-foreground">Similar Books</h2>
          
          {isLoading ? (
            <div className="flex items-center justify-center py-12 sm:py-24">
              <div className="text-center">
                <div className="inline-block animate-spin rounded-full h-8 w-8 sm:h-10 sm:w-10 border-2 border-primary border-t-transparent mb-3 sm:mb-4"></div>
                <p className="text-muted-foreground text-base sm:text-lg">Mencari buku yang serupa...</p>
              </div>
            </div>
          ) : results.length > 0 ? (
            <>
              <p className="text-sm sm:text-base text-muted-foreground mb-4 sm:mb-8">
                Ditemukan {results.length} buku yang serupa
              </p>
              <div className="grid grid-cols-2 sm:grid-cols-3 md:grid-cols-4 lg:grid-cols-5 gap-2 sm:gap-3 lg:gap-4">
                {results.map((book) => (
                  <BookCard
                    key={book.id}
                    id={book.id}
                    title={book.title}
                    cover={book.cover}
                    txt=""
                    score={book.score}
                  />
                ))}
              </div>
            </>
          ) : (
            <div className="flex items-center justify-center py-12 sm:py-24">
              <p className="text-muted-foreground text-base sm:text-lg text-center px-4">
                Tidak ada buku yang serupa dengan gambar yang diupload
              </p>
            </div>
          )}
        </div>
      </div>
    </main>
  );
}
