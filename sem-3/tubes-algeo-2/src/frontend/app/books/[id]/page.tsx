import Image from "next/image";
import Link from "next/link";

const API_URL = process.env.NEXT_PUBLIC_API_URL || "http://localhost:8000";
const INTERNAL_API_URL = process.env.INTERNAL_API_URL || "http://localhost:8000";
const APP_URL = process.env.NEXT_PUBLIC_APP_URL || "http://localhost:3000";

interface Book {
  id: string;
  title: string;
  cover?: string;
  text?: string;
}

async function getBook(id: string): Promise<Book> {
  const res = await fetch(`${APP_URL}/api/books/${id}`, { cache: "no-store" });
  if (!res.ok) throw new Error("Book not found");
  const response = await res.json();
  if (!response.success) throw new Error(response.error);
  return response.data;
}

async function getRecommendations(id: string): Promise<Book[]> {
  try {
    const res = await fetch(`${INTERNAL_API_URL}/recommend?book_id=${id}&top_k=5`, { cache: "no-store" });
    if (!res.ok) return [];
    
    const response = await res.json();
    if (!response.results) return [];
    
    // Map recommendations to Book format with cover URLs (use public API_URL for browser-accessible images)
    return response.results.map((rec: any) => ({
      id: rec.id,
      title: rec.title,
      cover: `${API_URL}/covers/${rec.id}.jpg`,
      score: rec.score,
    }));
  } catch {
    return [];
  }
}

export default async function BookDetail({
  params,
}: {
  params: Promise<{ id: string }>;
}) {
  const { id } = await params;

  let book: { id: string; title: string; cover?: string; text?: string } | null = null;

  try {
    book = await getBook(id);
  } catch (err) {
    return (
      <main className="min-h-screen w-full flex justify-center bg-background">
        <div className="w-full max-w-7xl flex items-center justify-center">
          <div className="text-center">
            <svg className="w-16 h-16 text-muted-foreground mx-auto mb-4 opacity-50" fill="none" stroke="currentColor" viewBox="0 0 24 24">
              <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={1.5} d="M12 6.253v13m0-13C10.832 5.477 9.246 5 7.5 5S4.168 5.477 3 6.253v13C4.168 18.477 5.754 18 7.5 18s3.332.477 4.5 1.253m0-13C13.168 5.477 14.754 5 16.5 5c1.747 0 3.332.477 4.5 1.253v13C19.832 18.477 18.247 18 16.5 18c-1.746 0-3.332.477-4.5 1.253" />
            </svg>
            <p className="text-xl text-muted-foreground">Buku tidak ditemukan</p>
          </div>
        </div>
      </main>
    );
  }

  let textContent = "[Unable to load text file]";
  if (book.text) {
    try {
      const textRes = await fetch(`${INTERNAL_API_URL}/${book.text}`, { cache: "no-store" });
      if (textRes.ok) textContent = await textRes.text();
    } catch {
      // TODO: message
    }
  }

  // Get recommendations from LSA-based API
  const recommendedBooks = await getRecommendations(id);

  return (
    <main className="min-h-screen w-full flex justify-center bg-background">
      <div className="w-full max-w-7xl px-3 sm:px-4">
        <div className="flex flex-col lg:flex-row gap-4 sm:gap-6 lg:gap-8 py-4 sm:py-6 lg:py-8">
          <div className="shrink-0 w-full lg:w-80">
            <div className="bg-card rounded-xl sm:rounded-2xl shadow-lg shadow-primary/5 p-4 sm:p-6 lg:sticky lg:top-20 overflow-hidden border border-border">
              <div className="w-full max-w-[200px] sm:max-w-[250px] lg:max-w-none mx-auto aspect-2/3 relative mb-4 sm:mb-6 overflow-hidden rounded-lg sm:rounded-xl border border-border">
                {book.cover ? (
                  <Image
                    src={book.cover}
                    alt={book.title}
                    fill
                    className="object-cover"
                    unoptimized
                  />
                ) : (
                  <div className="w-full h-full flex items-center justify-center bg-secondary text-muted-foreground">
                    <svg className="w-16 h-16 opacity-50" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                      <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={1.5} d="M12 6.253v13m0-13C10.832 5.477 9.246 5 7.5 5S4.168 5.477 3 6.253v13C4.168 18.477 5.754 18 7.5 18s3.332.477 4.5 1.253m0-13C13.168 5.477 14.754 5 16.5 5c1.747 0 3.332.477 4.5 1.253v13C19.832 18.477 18.247 18 16.5 18c-1.746 0-3.332.477-4.5 1.253" />
                    </svg>
                  </div>
                )}
                <div className="absolute top-2 left-2 px-2 py-1 bg-white/80 backdrop-blur-sm rounded-md text-xs font-medium text-foreground/70">
                  #{id}
                </div>
              </div>

              <h1 className="text-sm sm:text-base lg:text-lg font-bold text-center break-words leading-tight text-foreground">{book.title}</h1>
            </div>
          </div>

          <div className="flex-1 min-w-0">
            <div className="bg-card rounded-xl sm:rounded-2xl shadow-lg shadow-primary/5 p-4 sm:p-6 lg:p-8 overflow-y-auto max-h-[400px] sm:max-h-[500px] lg:max-h-[600px] border border-border">
              <h2 className="text-lg sm:text-xl font-semibold mb-3 sm:mb-4 text-foreground">Content</h2>

              <h3 className="text-xs sm:text-sm uppercase tracking-wide text-muted-foreground mb-4 sm:mb-6 font-medium">
                {book.title}
              </h3>

              <pre className="whitespace-pre-wrap text-foreground/80 bg-secondary/50 p-3 sm:p-4 rounded-lg sm:rounded-xl border border-border leading-relaxed font-mono text-xs sm:text-sm">
                {textContent}
              </pre>
            </div>
          </div>
        </div>

        <div className="py-6 sm:py-8 lg:py-12">
          <h2 className="text-xl sm:text-2xl font-bold mb-4 sm:mb-6 lg:mb-8 text-foreground">Recommended Books</h2>

          <div className="overflow-x-auto pb-4 -mx-2 px-2 scrollbar-hide">
            <div className="flex gap-3 sm:gap-4 lg:gap-6 min-w-min pt-2">
              {recommendedBooks.length > 0 ? (
                recommendedBooks.map((recBook: any) => (
                  <Link key={recBook.id} href={`/books/${recBook.id}`}>
                    <div className="group shrink-0 w-32 sm:w-40 lg:w-48 cursor-pointer bg-card hover:shadow-xl hover:shadow-primary/10 hover:-translate-y-1 transition-all duration-300 rounded-lg sm:rounded-xl overflow-hidden border border-border hover:border-primary/30">
                      <div className="w-full aspect-2/3 relative overflow-hidden">
                        {recBook.cover ? (
                          <Image
                            src={recBook.cover}
                            alt={recBook.title}
                            fill
                            className="object-cover"
                            unoptimized
                          />
                        ) : (
                          <div className="w-full h-full flex items-center justify-center bg-secondary text-muted-foreground">
                            <svg className="w-12 h-12 opacity-50" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                              <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={1.5} d="M12 6.253v13m0-13C10.832 5.477 9.246 5 7.5 5S4.168 5.477 3 6.253v13C4.168 18.477 5.754 18 7.5 18s3.332.477 4.5 1.253m0-13C13.168 5.477 14.754 5 16.5 5c1.747 0 3.332.477 4.5 1.253v13C19.832 18.477 18.247 18 16.5 18c-1.746 0-3.332.477-4.5 1.253" />
                            </svg>
                          </div>
                        )}
                        <div className="absolute top-2 left-2 px-2 py-1 bg-white/80 backdrop-blur-sm rounded-md text-xs font-medium text-foreground/70">
                          #{recBook.id}
                        </div>
                        {recBook.score !== undefined && (
                          <div className="absolute top-2 right-2 px-2 py-1 bg-primary/90 backdrop-blur-sm rounded-md text-xs font-medium text-primary-foreground">
                            {(recBook.score * 100).toFixed(1)}%
                          </div>
                        )}
                        <div className="absolute inset-0 bg-gradient-to-t from-black/80 via-black/40 to-transparent opacity-0 group-hover:opacity-100 transition-opacity duration-300 flex items-end p-4">
                          <h3 className="font-semibold text-sm text-white line-clamp-3">{recBook.title}</h3>
                        </div>
                      </div>
                    </div>
                  </Link>
                ))
              ) : (
                <p className="text-muted-foreground">Tidak ada rekomendasi buku</p>
              )}
            </div>
          </div>
        </div>
      </div>
    </main>
  );
}
