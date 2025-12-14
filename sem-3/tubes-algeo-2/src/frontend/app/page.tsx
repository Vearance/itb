import { BookCard } from "@/components/book-card";
import { Pagination } from "@/components/pagination";

const BOOKS_PER_PAGE = 10;

interface Book {
  id: string;
  title: string;
  cover?: string;
  txt?: string;
}

async function fetchBooks(page: number = 1) {
  try {
    const res = await fetch(
      `${process.env.NEXT_PUBLIC_APP_URL || "http://localhost:3000"}/api/books?page=${page}&limit=${BOOKS_PER_PAGE}`,
      { cache: "no-store" }
    );

    if (!res.ok) {
      throw new Error("Failed to fetch books");
    }

    const response = await res.json();
    
    if (!response.success) {
      throw new Error(response.error || "Failed to fetch books");
    }

    return {
      books: response.data as Book[],
      totalBooks: response.pagination.totalBooks,
      totalPages: response.pagination.totalPages,
      currentPage: response.pagination.currentPage,
    };
  } catch (error) {
    console.error("Error fetching books:", error);
    return {
      books: [],
      totalBooks: 0,
      totalPages: 0,
      currentPage: 1,
    };
  }
}

export default async function Home({
  searchParams,
}: {
  searchParams: Promise<{ page?: string }>;
}) {
  const params = await searchParams;
  const page = parseInt(params.page || "1", 10);

  const { books, totalPages, currentPage } = await fetchBooks(page);

  return (
    <main className="min-h-screen w-full px-3 sm:px-4 py-4 sm:py-6 lg:py-8 flex justify-center bg-background">
      <div className="w-full max-w-6xl">
        <div className="grid grid-cols-2 sm:grid-cols-3 md:grid-cols-4 lg:grid-cols-5 gap-2 sm:gap-3 lg:gap-4">
          {books.map((book) => (
            <BookCard
              key={book.id}
              id={book.id}
              title={book.title}
              cover={book.cover}
              txt={book.txt}
            />
          ))}
        </div>
        <Pagination currentPage={currentPage} totalPages={totalPages} />
      </div>
    </main>
  );
}
