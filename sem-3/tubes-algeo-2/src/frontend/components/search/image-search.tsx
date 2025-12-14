'use client';

interface Book {
  id: string;
  title: string;
  cover?: string;
  score?: number;
}

export async function performImageSearch(imageBase64: string): Promise<Book[]> {
  try {
    const APP_URL = process.env.NEXT_PUBLIC_API_URL || "http://localhost:8000";
    const formData = new FormData();
    
    try {
      const parts = imageBase64.split(',');
      const base64Data = parts.length > 1 ? parts[1] : parts[0];
      const mimeMatch = parts[0].match(/:(.*?);/);
      const mimeType = mimeMatch ? mimeMatch[1] : 'image/jpeg';
      const byteCharacters = atob(base64Data);
      const byteNumbers = new Array(byteCharacters.length);
      for (let i = 0; i < byteCharacters.length; i++) {
        byteNumbers[i] = byteCharacters.charCodeAt(i);
      }
      const byteArray = new Uint8Array(byteNumbers);
      const blob = new Blob([byteArray], { type: mimeType });
      
      formData.append('file', blob);
    } catch (parseError) {
      console.error("Error parsing image:", parseError);
      throw new Error("Invalid image format");
    }
    
    const res = await fetch(`${APP_URL}/search/image`, {
      method: 'POST',
      body: formData,
    });

    if (!res.ok) {
      throw new Error(`Failed to search by image: ${res.status}`);
    }

    const response = await res.json();

    if (response.success && Array.isArray(response.data)) {
      const books: Book[] = response.data.map((item: any) => {
        return {
          id: item.id,
          title: item.title || `Book ${item.id}`,
          cover: `${APP_URL}/covers/${item.id}.jpg`,
          score: item.similarity_score,
        };
      });
      return books;
    }
    return [];
  } catch (error) {
    console.error("Error searching by image:", error);
    return [];
  }
}
