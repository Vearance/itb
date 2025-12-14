"use client"

import Link from "next/link";
import Image from "next/image";
import { SearchBar } from "@/components/search/search-bar";

export default function Header() {
    return (
        <header className="sticky top-0 z-50 bg-background border-b border-border flex items-center justify-between gap-2 sm:gap-4 py-2 sm:py-3 px-2 sm:px-4">
            <div className="flex-shrink-0 flex justify-start">
                <Link href="/" className="flex items-center gap-3">
                    <Image
                        src="/logo-itb.png"
                        alt="Logo"
                        width={40}
                        height={40}
                        className="object-contain w-8 h-8 sm:w-10 sm:h-10"
                    />
                
                    <Image
                        src="/header.jpg"
                        alt="Logo"
                        width={1280}
                        height={698}
                        className="object-contain h-8 sm:h-10 w-auto"
                    />
                </Link>
            </div>

            <div className="flex-1 flex items-center justify-center relative min-w-0">
                <SearchBar />
            </div>

            <div className="hidden sm:flex flex-shrink-0 w-10">
            </div>
        </header>
    );
}
