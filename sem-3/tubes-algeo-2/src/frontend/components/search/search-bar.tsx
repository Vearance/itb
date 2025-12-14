"use client"

import { Input } from "@/components/ui/input";
import { Search, Image as ImageIcon, FileText, Plus } from "lucide-react";
import { Button } from "@/components/ui/button";
import { useRef, useState, useCallback } from "react";
import { useRouter, usePathname } from "next/navigation";
import { QueryResult } from "./query-result";
import {
  DropdownMenu,
  DropdownMenuTrigger,
  DropdownMenuContent,
  DropdownMenuItem,
} from "@/components/ui/dropdown-menu";

interface Book {
  id: string;
  title: string;
  cover?: string;
}

type UploadModalType = "image" | "document" | null;

export function SearchBar() {
  const fileInputRef = useRef<HTMLInputElement>(null);
  const docFileInputRef = useRef<HTMLInputElement>(null);
  const [searchQuery, setSearchQuery] = useState("");
  const [searchResults, setSearchResults] = useState<Book[]>([]);
  const [showDropdown, setShowDropdown] = useState(false);
  const [selectedIndex, setSelectedIndex] = useState(-1);
  const [isSearching, setIsSearching] = useState(false);
  const [hoveredIcon, setHoveredIcon] = useState<"search" | "plus" | null>(null);
  const [openDropdown, setOpenDropdown] = useState(false);
  const [uploadModal, setUploadModal] = useState<UploadModalType>(null);
  const [isDragging, setIsDragging] = useState(false);
  const router = useRouter();
  const pathname = usePathname();

  const performSearch = useCallback(async (query: string) => {
    if (!query.trim()) {
      setSearchResults([]);
      return;
    }

    setIsSearching(true);
    try {
      const params = new URLSearchParams({ q: query, type: "title" });
      const res = await fetch(`/api/search?${params}`);

      if (!res.ok) {
        throw new Error("Failed to search books");
      }

      const response = await res.json();

      if (response.success) {
        setSearchResults(response.data);
      }
    } catch (error) {
      console.error("Error searching books:", error);
      setSearchResults([]);
    } finally {
      setIsSearching(false);
    }
  }, []);

  const handleSearchChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    const query = e.target.value;
    setSearchQuery(query);
    setSelectedIndex(-1);

    if (query.trim() === "") {
      setSearchResults([]);
      setShowDropdown(false);
    } else {
      performSearch(query);
      setShowDropdown(true);
    }
  };

  const handleKeyDown = (e: React.KeyboardEvent<HTMLInputElement>) => {
    if (e.key === "ArrowDown") {
      e.preventDefault();
      if (searchResults.length > 0) {
        setSelectedIndex((prev) =>
          prev < searchResults.length - 1 ? prev + 1 : prev
        );
        setShowDropdown(true);
      }
    }
    if (e.key === "ArrowUp") {
      e.preventDefault();
      if (searchResults.length > 0) {
        setSelectedIndex((prev) => (prev > 0 ? prev - 1 : -1));
      }
    }

    if (e.key === "Enter") {
      e.preventDefault();
      if (selectedIndex >= 0 && searchResults.length > 0) {
        handleBookSelect(searchResults[selectedIndex].id);
      } else {
        handleSearchSubmit(e as any);
      }
    }
    if (e.key === "Escape") {
      setShowDropdown(false);
      setSelectedIndex(-1);
    }
  };

  const handleBookSelect = (bookId: string) => {
    setSearchQuery("");
    setShowDropdown(false);
    setSelectedIndex(-1);
    router.push(`/books/${bookId}`);
  };

  const handleSearchSubmit = (e: React.FormEvent) => {
    e.preventDefault();
    if (searchQuery.trim()) {
      const query = encodeURIComponent(searchQuery);
      setSearchQuery("");
      setShowDropdown(false);
      setSelectedIndex(-1);
      router.push(`/search?q=${query}&type=title`);
    }
  };

  const handleImageUpload = (file: File) => {
    if (file && file.type.startsWith("image/")) {
      const reader = new FileReader();
      reader.onload = (e) => {
        const imageData = e.target?.result as string;
        sessionStorage.setItem("uploadedImage", imageData);
        
        if (pathname === "/search/image") {
          window.dispatchEvent(new CustomEvent("newImageUpload", { detail: imageData }));
        } else {
          router.push(`/search/image`);
        }
      };
      reader.readAsDataURL(file);
    }
  };

  const handleDocUpload = (file: File) => {
    if (file && file.name.endsWith(".txt")) {
      const reader = new FileReader();
      reader.onload = (e) => {
        const textContent = e.target?.result as string;
        const docData = {
          name: file.name,
          content: textContent
        };
        sessionStorage.setItem("uploadedDoc", JSON.stringify(docData));
        
        if (pathname === "/search/doc") {
          window.dispatchEvent(new CustomEvent("newDocUpload", { detail: docData }));
        } else {
          router.push(`/search/doc`);
        }
      };
      reader.readAsText(file);
    } else {
      alert("Please upload a .txt file");
    }
  };

  const imageUpload = (e: React.ChangeEvent<HTMLInputElement>) => {
    const file = e.target.files?.[0];
    if (file) {
      handleImageUpload(file);
      setUploadModal(null);
    }
  };

  const docUpload = (e: React.ChangeEvent<HTMLInputElement>) => {
    const file = e.target.files?.[0];
    if (file) {
      handleDocUpload(file);
      setUploadModal(null);
    }
  };

  const handleDragOver = (e: React.DragEvent) => {
    e.preventDefault();
    setIsDragging(true);
  };

  const handleDragLeave = (e: React.DragEvent) => {
    e.preventDefault();
    setIsDragging(false);
  };

  const handleDrop = (e: React.DragEvent) => {
    e.preventDefault();
    setIsDragging(false);
    const file = e.dataTransfer.files?.[0];
    if (file) {
      if (uploadModal === "image") {
        handleImageUpload(file);
      } else if (uploadModal === "document") {
        handleDocUpload(file);
      }
    }
    setUploadModal(null);
  };

  const handleUploadAreaClick = () => {
    if (uploadModal === "image") {
      fileInputRef.current?.click();
    } else if (uploadModal === "document") {
      docFileInputRef.current?.click();
    }
  };

  return (
    <div className="w-full max-w-[500px] relative">
      <form onSubmit={handleSearchSubmit} className="w-full relative">
        <div className="flex items-center bg-card rounded-xl border border-border w-full h-9 sm:h-11 overflow-hidden shadow-sm">
          <div className="relative">
            <div
              onMouseEnter={() => setHoveredIcon("search")}
              onMouseLeave={() => setHoveredIcon(null)}
              className="text-muted-foreground shrink-0 ml-2 sm:ml-3 flex items-center"
            >
              <Search size={18} className="sm:w-5 sm:h-5" />
            </div>
          </div>
          <Input
            type="text"
            placeholder="Cari judul..."
            value={searchQuery}
            onChange={handleSearchChange}
            onKeyDown={handleKeyDown}
            onFocus={() => searchQuery && setShowDropdown(true)}
            className="bg-transparent text-foreground placeholder-muted-foreground border-none focus-visible:ring-0 focus-visible:ring-offset-0 flex-1 text-sm sm:text-base px-2 sm:px-3"
          />
          <DropdownMenu open={openDropdown} onOpenChange={(open) => {
            if (uploadModal) {
              setUploadModal(null);
              return;
            }
            setOpenDropdown(open);
          }}>
            <DropdownMenuTrigger asChild>
              <Button
                type="button"
                variant="ghost"
                size="icon"
                onMouseEnter={() => setHoveredIcon("plus")}
                onMouseLeave={() => setHoveredIcon(null)}
                className="hover:bg-secondary rounded-none h-full px-3 relative shrink-0"
              >
                <Plus className="text-muted-foreground" size={20} />
              </Button>
            </DropdownMenuTrigger>
            <DropdownMenuContent align="end" className="w-48 bg-card border-border">
                <DropdownMenuItem
                  onClick={() => {
                    setUploadModal("image");
                    setOpenDropdown(false);
                  }}
                  className="flex items-center gap-2 cursor-pointer hover:bg-secondary focus:bg-secondary"
                >
                  <ImageIcon size={18} className="text-primary" />
                  <span>Cari berdasarkan gambar</span>
                </DropdownMenuItem>
                <DropdownMenuItem
                  onClick={() => {
                    setUploadModal("document");
                    setOpenDropdown(false);
                  }}
                  className="flex items-center gap-2 cursor-pointer hover:bg-secondary focus:bg-secondary"
                >
                  <FileText size={18} className="text-primary" />
                  <span>Cari berdasarkan dokumen</span>
                </DropdownMenuItem>
              </DropdownMenuContent>
            </DropdownMenu>
          {hoveredIcon === "plus" && (
            <div className="absolute top-12 right-2 px-3 py-1.5 bg-foreground text-background text-xs rounded-lg whitespace-nowrap z-50 animate-in fade-in duration-200">
              Opsi pencarian
              <div className="absolute bottom-full right-4 w-0 h-0 border-l-4 border-r-4 border-b-4 border-l-transparent border-r-transparent border-b-foreground"></div>
            </div>
          )}
        </div>

        <QueryResult
          results={searchResults.slice(0, 5)}
          totalResults={searchResults.length}
          selectedIndex={selectedIndex}
          onSelect={handleBookSelect}
          showDropdown={showDropdown}
          searchQuery={searchQuery}
        />
      </form>

      {uploadModal && (
        <div 
          onClick={handleUploadAreaClick}
          onDragOver={handleDragOver}
          onDragLeave={handleDragLeave}
          onDrop={handleDrop}
          className={`absolute top-full left-0 right-0 mt-2 bg-card border-2 border-dashed rounded-xl shadow-lg z-50 p-8 cursor-pointer transition-colors ${
            isDragging 
              ? "bg-primary/10 border-primary" 
              : "border-border hover:bg-secondary hover:border-primary/50"
          }`}
        >
          <div className="flex flex-col items-center gap-2">
            {uploadModal === "image" ? (
              <ImageIcon className="text-primary" size={24} />
            ) : (
              <FileText className="text-primary" size={24} />
            )}
            <p className="text-center text-muted-foreground text-sm">
              {uploadModal === "image" 
                ? "Klik atau drag gambar untuk upload" 
                : "Klik atau drag dokumen (.txt) untuk upload"}
            </p>
          </div>
        </div>
      )}

      {hoveredIcon === "search" && (
        <div className="absolute top-12 left-2 px-3 py-1.5 bg-foreground text-background text-xs rounded-lg whitespace-nowrap z-50 animate-in fade-in duration-200">
          Cari berdasarkan judul
          <div className="absolute bottom-full left-4 w-0 h-0 border-l-4 border-r-4 border-b-4 border-l-transparent border-r-transparent border-b-foreground"></div>
        </div>
      )}

      <input
        ref={fileInputRef}
        type="file"
        accept="image/*"
        onChange={imageUpload}
        className="hidden"
      />
      <input
        ref={docFileInputRef}
        type="file"
        accept=".txt"
        onChange={docUpload}
        className="hidden"
      />
    </div>
  );
}
