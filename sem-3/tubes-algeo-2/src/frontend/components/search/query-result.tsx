"use client"

import { SearchResultCard } from "./search-result-card";

interface QueryResultProps {
  results: Array<{ id: string; title: string; cover?: string }>;
  totalResults: number;
  selectedIndex: number;
  onSelect: (id: string) => void;
  showDropdown: boolean;
  searchQuery: string;
}

export function QueryResult({ results, totalResults, selectedIndex, onSelect, showDropdown, searchQuery }: QueryResultProps) {
  if (!showDropdown || totalResults === 0) return null;

  const hasMoreResults = totalResults > 5;

  return (
    <div className="absolute top-full left-0 right-0 mt-2 bg-card border border-border rounded-xl shadow-lg shadow-primary/5 z-50 overflow-hidden">
      {results.map((book, index) => (
        <SearchResultCard
          key={book.id}
          book={book}
          isSelected={selectedIndex === index}
          onSelect={onSelect}
        />
      ))}
      {hasMoreResults && (
        <div className="px-4 py-3 bg-secondary/50 border-t border-border">
          <a
            href={`/search?q=${encodeURIComponent(searchQuery)}&type=title`}
            className="text-primary hover:text-primary/80 text-sm font-medium transition-colors"
          >
            Lihat semua hasil ({totalResults})
          </a>
        </div>
      )}
    </div>
  );
}
