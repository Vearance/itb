"use client"

import Image from "next/image";

interface SearchResultCardProps {
  book: {
    id: string;
    title: string;
    cover?: string;
  };
  isSelected: boolean;
  onSelect: (id: string) => void;
}

export function SearchResultCard({ book, isSelected, onSelect }: SearchResultCardProps) {
  return (
    <button
      onClick={() => onSelect(book.id)}
      className={`w-full flex items-center gap-3 px-4 py-3 transition-colors border-b last:border-b-0 border-border ${
        isSelected ? "bg-primary/10" : "hover:bg-secondary"
      }`}
    >
      <div className="flex-shrink-0 w-10 h-14 bg-secondary rounded-md border border-border flex items-center justify-center overflow-hidden">
        {book.cover ? (
          <Image
            src={book.cover}
            alt={book.title}
            width={40}
            height={56}
            className="object-cover"
            unoptimized
          />
        ) : (
          <svg className="w-5 h-5 text-muted-foreground opacity-50" fill="none" stroke="currentColor" viewBox="0 0 24 24">
            <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={1.5} d="M12 6.253v13m0-13C10.832 5.477 9.246 5 7.5 5S4.168 5.477 3 6.253v13C4.168 18.477 5.754 18 7.5 18s3.332.477 4.5 1.253m0-13C13.168 5.477 14.754 5 16.5 5c1.747 0 3.332.477 4.5 1.253v13C19.832 18.477 18.247 18 16.5 18c-1.746 0-3.332.477-4.5 1.253" />
          </svg>
        )}
      </div>
      <div className="flex-1 text-left">
        <span className="text-sm text-foreground font-medium">{book.title}</span>
        <span className="text-xs text-muted-foreground ml-2">#{book.id}</span>
      </div>
    </button>
  );
}
