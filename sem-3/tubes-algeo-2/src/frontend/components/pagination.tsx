"use client";

import Link from "next/link";
import { ChevronLeft, ChevronRight } from "lucide-react";

interface PaginationProps {
  currentPage: number;
  totalPages: number;
}

export function Pagination({ currentPage, totalPages }: PaginationProps) {
  if (totalPages <= 1) return null;

  const pages = [];
  const maxPagesToShow = 5;
  let startPage = Math.max(1, currentPage - Math.floor(maxPagesToShow / 2));
  let endPage = Math.min(totalPages, startPage + maxPagesToShow - 1);

  if (endPage - startPage + 1 < maxPagesToShow) {
    startPage = Math.max(1, endPage - maxPagesToShow + 1);
  }

  if (startPage > 1) {
    pages.push(1);
    if (startPage > 2) {
      pages.push("...");
    }
  }

  for (let i = startPage; i <= endPage; i++) {
    pages.push(i);
  }

  if (endPage < totalPages) {
    if (endPage < totalPages - 1) {
      pages.push("...");
    }
    pages.push(totalPages);
  }

  return (
    <div className="flex justify-center items-center gap-1 sm:gap-2 mt-8 sm:mt-12">
      <Link
        href={`?page=${Math.max(1, currentPage - 1)}`}
        className={`p-1.5 sm:p-2 rounded-lg transition-colors ${
          currentPage === 1
            ? "text-muted-foreground cursor-not-allowed"
            : "hover:bg-secondary text-foreground"
        }`}
        onClick={(e) => currentPage === 1 && e.preventDefault()}
      >
        <ChevronLeft className="w-4 h-4 sm:w-5 sm:h-5" />
      </Link>

      {pages.map((page, index) => (
        <div key={index}>
          {page === "..." ? (
            <span className="px-2 sm:px-3 py-1.5 sm:py-2 text-muted-foreground text-sm sm:text-base">...</span>
          ) : (
            <Link
              href={`?page=${page}`}
              className={`px-2.5 sm:px-3 py-1.5 sm:py-2 rounded-lg transition-colors font-medium text-sm sm:text-base ${
                currentPage === page
                  ? "bg-primary text-primary-foreground"
                  : "hover:bg-secondary text-foreground"
              }`}
            >
              {page}
            </Link>
          )}
        </div>
      ))}

      <Link
        href={`?page=${Math.min(totalPages, currentPage + 1)}`}
        className={`p-1.5 sm:p-2 rounded-lg transition-colors ${
          currentPage === totalPages
            ? "text-muted-foreground cursor-not-allowed"
            : "hover:bg-secondary text-foreground"
        }`}
        onClick={(e) => currentPage === totalPages && e.preventDefault()}
      >
        <ChevronRight className="w-4 h-4 sm:w-5 sm:h-5" />
      </Link>
    </div>
  );
}
