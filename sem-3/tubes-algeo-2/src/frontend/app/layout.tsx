import type { Metadata } from "next";
import { sfmono } from '@/lib/fonts';
import { cn } from '@/lib/utils';
import Header from "@/components/header";
import Footer from "@/components/footer";
import "./globals.css";

export const metadata: Metadata = {
  title: "ITBook",
  description: "Inspirational, Timeless, Books!",
};

export default function RootLayout({
  children,
}: Readonly<{
  children: React.ReactNode;
}>) {
  return (
    <html lang="en">
      <body
        className={cn(
          sfmono.variable,
          sfmono.className,
          "antialiased font-normal",
          'bg-background',
        )}
      >
        <Header />
        {children}
        <Footer />
      </body>
    </html>
  );
}
