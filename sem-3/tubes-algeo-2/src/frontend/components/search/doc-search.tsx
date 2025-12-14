'use client';

interface Book {
    id: string;
    title: string;
    cover?: string;
    score?: number;
}

export async function performDocumentSearch(file: File): Promise<Book[]> {
    try {
        const APP_URL = process.env.NEXT_PUBLIC_API_URL || "http://localhost:8000";
        const formData = new FormData();
        formData.append('file', file);

        const res = await fetch(`${APP_URL}/search/document`, {
            method: 'POST',
            body: formData,
        });

        if (!res.ok) {
            throw new Error(`Failed to search by document: ${res.status}`);
        }

        const response = await res.json();

        if (response.success && Array.isArray(response.data)) {
            const books: Book[] = response.data.map((item: any) => {
                return {
                    id: item.id,
                    title: item.title || `Book ${item.id}`,
                    cover: `${APP_URL}/covers/${item.id}.jpg`,
                    score: item.score,
                };
            });
            return books;
        }
        return [];
    } catch (error) {
        console.error("Error searching by document:", error);
        return [];
    }
}
