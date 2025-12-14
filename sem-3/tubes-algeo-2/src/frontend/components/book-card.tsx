import Image from "next/image";
import Link from "next/link";

export function BookCard({
    id,
    title,
    cover,
    txt,
    score,
}: {
    id: string;
    title: string;
    cover?: string;
    txt?: string;
    score?: number;
}) {
    return (
        <Link href={`/books/${id}`}>
            <div className="group bg-card hover:shadow-xl hover:shadow-primary/10 hover:-translate-y-1 transition-all duration-300 w-full overflow-hidden rounded-lg sm:rounded-xl border border-border hover:border-primary/30 cursor-pointer">
                <div className="w-full aspect-[2/3] relative flex-shrink-0 overflow-hidden">
                    {cover ? (
                        <Image
                            src={cover}
                            alt={title}
                            fill
                            className="object-cover"
                        />
                    ) : (
                        <div className="w-full h-full flex items-center justify-center bg-secondary text-muted-foreground">
                            <svg className="w-8 h-8 sm:w-12 sm:h-12 opacity-50" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                                <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={1.5} d="M12 6.253v13m0-13C10.832 5.477 9.246 5 7.5 5S4.168 5.477 3 6.253v13C4.168 18.477 5.754 18 7.5 18s3.332.477 4.5 1.253m0-13C13.168 5.477 14.754 5 16.5 5c1.747 0 3.332.477 4.5 1.253v13C19.832 18.477 18.247 18 16.5 18c-1.746 0-3.332.477-4.5 1.253" />
                            </svg>
                        </div>
                    )}
                    {/* Book ID badge */}
                    <div className="absolute top-1 left-1 sm:top-2 sm:left-2 px-1.5 py-0.5 sm:px-2 sm:py-1 bg-white/80 backdrop-blur-sm rounded-md text-[10px] sm:text-xs font-medium text-foreground/70">
                        #{id}
                    </div>
                    {/* Score badge */}
                    {score !== undefined && (
                        <div className="absolute top-1 right-1 sm:top-2 sm:right-2 px-1.5 py-0.5 sm:px-2 sm:py-1 bg-primary/90 backdrop-blur-sm rounded-md text-[10px] sm:text-xs font-medium text-primary-foreground">
                            {(score * 100).toFixed(1)}%
                        </div>
                    )}
                    {/* Title overlay on hover */}
                    <div className="absolute inset-0 bg-gradient-to-t from-black/80 via-black/40 to-transparent opacity-0 group-hover:opacity-100 transition-opacity duration-300 flex items-end p-2 sm:p-4">
                        <h3 className="font-semibold text-xs sm:text-sm text-white line-clamp-3">{title}</h3>
                    </div>
                </div>
            </div>
        </Link>
    );
}
