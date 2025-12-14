'use client';

import { BookCard } from "@/components/book-card";
import { useEffect, useState, useCallback } from "react";
import { FileText } from "lucide-react";
import { performDocumentSearch } from "@/components/search/doc-search";

interface Book {
    id: string;
    title: string;
    cover?: string;
    score?: number;
}

export default function DocSearchPage() {
    const [results, setResults] = useState<Book[]>([]);
    const [isLoading, setIsLoading] = useState(false);
    const [uploadedDoc, setUploadedDoc] = useState<{ name: string; content: string } | null>(null);

    const handleDocSearch = useCallback(async (docName: string, docContent: string) => {
        setIsLoading(true);
        try {
            const blob = new Blob([docContent], { type: 'text/plain' });
            const file = new File([blob], docName, { type: 'text/plain' });

            const books = await performDocumentSearch(file);
            setResults(books);
        } finally {
            setIsLoading(false);
        }
    }, []);

    useEffect(() => {
        const storedDoc = sessionStorage.getItem("uploadedDoc");
        if (storedDoc) {
            try {
                const parsedDoc = JSON.parse(storedDoc);
                setUploadedDoc(parsedDoc);
                handleDocSearch(parsedDoc.name, parsedDoc.content);
                sessionStorage.removeItem("uploadedDoc");
            } catch (e) {
                console.error("Error parsing stored document:", e);
            }
        }
    }, [handleDocSearch]);

    useEffect(() => {
        const handleNewUpload = (event: CustomEvent<{ name: string; content: string }>) => {
            const docData = event.detail;
            setUploadedDoc(docData);
            handleDocSearch(docData.name, docData.content);
            sessionStorage.removeItem("uploadedDoc");
        };

        window.addEventListener("newDocUpload", handleNewUpload as EventListener);
        return () => {
            window.removeEventListener("newDocUpload", handleNewUpload as EventListener);
        };
    }, [handleDocSearch]);

    if (!uploadedDoc) {
        return (
            <main className="min-h-screen w-full px-3 sm:px-4 py-4 sm:py-8 flex justify-center bg-background">
                <div className="w-full max-w-7xl flex items-center justify-center py-12 sm:py-24">
                    <p className="text-muted-foreground text-base sm:text-lg text-center px-4">
                        Gunakan upload document untuk mencari buku berdasarkan isi dokumen.
                    </p>
                </div>
            </main>
        );
    }

    return (
        <main className="min-h-screen w-full bg-background">
            <div className="w-full px-3 sm:px-4 py-4 sm:py-8 flex justify-center">
                <div className="w-full max-w-3xl">
                    <div className="rounded-xl sm:rounded-2xl overflow-hidden border border-border shadow-lg shadow-primary/5 bg-card p-4 sm:p-6">
                        <div className="flex items-center gap-2 sm:gap-3 mb-3 sm:mb-4 pb-3 sm:pb-4 border-b border-border">
                            <div className="p-1.5 sm:p-2 bg-primary/10 rounded-lg">
                                <FileText className="text-primary w-5 h-5 sm:w-6 sm:h-6" />
                            </div>
                            <div className="min-w-0 flex-1">
                                <h3 className="font-semibold text-foreground text-sm sm:text-base truncate">{uploadedDoc.name}</h3>
                                <p className="text-xs sm:text-sm text-muted-foreground">Preview</p>
                            </div>
                        </div>
                        <div className="bg-secondary/50 rounded-lg sm:rounded-xl p-3 sm:p-4 max-h-40 sm:max-h-60 overflow-y-auto font-mono text-xs sm:text-sm text-foreground/80 whitespace-pre-wrap border border-border">
                            {uploadedDoc.content}
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
                                Tidak ada buku yang serupa dengan dokumen yang diupload
                            </p>
                        </div>
                    )}
                </div>
            </div>
        </main>
    );
}
