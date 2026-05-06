"use client";

import { ChevronRight, Globe, Plus, Search, Zap } from "lucide-react";
import { motion } from "motion/react";
import { useRouter } from "next/navigation";
import React from "react";
import {
  TraversalApiErrorResponse,
  TraversalApiResponse,
  TraversalMethod,
  TraversalRequestPayload,
} from "@/types/traversal";

const TRAVERSAL_API_URL = process.env.API_URL || "http://127.0.0.1:6767";

export default function Home() {
  return (
    <div className="min-h-screen flex flex-col selection:bg-primary selection:text-on-primary">
      {/* Global Overlays */}
      <div className="fixed inset-0 scanline-overlay z-100 pointer-events-none" />

      <HomeSidebar />

      <main className="flex-1 min-h-screen flex flex-col relative z-10">
        <Hero />
      </main>
    </div>
  )
}

function HomeSidebar() {
  const router = useRouter();
  const [formData, setFormData] = React.useState({
    url: "",
    selector: "",
    amount: 0,
    method: "DFS" as TraversalMethod,
  });
  const [isSubmitting, setIsSubmitting] = React.useState(false);
  const [submitError, setSubmitError] = React.useState<string | null>(null);

  const handleSubmit = async (e: React.SubmitEvent<HTMLFormElement>) => {
    e.preventDefault();

    if (isSubmitting) {
      return;
    }

    if (!formData.url.trim() || !formData.selector.trim()) {
      setSubmitError("URL and CSS selector are required.");
      return;
    }

    const payload: TraversalRequestPayload = {
      url: formData.url.trim(),
      selector: formData.selector.trim(),
      amount: formData.amount,
      type: formData.method,
    };

    try {
      setIsSubmitting(true);
      setSubmitError(null);

      const response = await fetch(TRAVERSAL_API_URL, {
        method: "POST",
        headers: {
          "Content-Type": "application/json"
        },
        body: JSON.stringify(payload)
      });

      if (!response.ok) {
        const maybeError = await response.json().catch(() => null) as TraversalApiErrorResponse | null;
        const fieldPrefix = maybeError?.field ? `${maybeError.field}: ` : "";
        throw new Error(`${fieldPrefix}${maybeError?.error ?? `Request failed (${response.status})`}`);
      }

      const result = (await response.json()) as TraversalApiResponse;

      sessionStorage.setItem("traversal:request", JSON.stringify(payload));
      sessionStorage.setItem("traversal:response", JSON.stringify(result));

      router.push("/visualizer");
    } catch (error) {
      if (error instanceof Error) {
        setSubmitError(error.message);
      } else {
        setSubmitError("Failed to submit traversal request.");
      }
    } finally {
      setIsSubmitting(false);
    }


  }

  return (
    <aside className="w-sm h-screen flex flex-col bg-background border-r border-primary/20 font-sans uppercase tracking-wider text-md">
      <div className="p-4 pb-4">
        <motion.span
          initial={{ opacity: 0 }}
          animate={{ opacity: 1 }}
          className="text-primary font-black italic text-xl tracking-tighter"
        >
          TRAVERSAL_CONFIG
        </motion.span>
        <div className="text-on-surface-variant mt-1 font-mono uppercase text-sm">
          maaf_ya_bang_aku_gak_ngerti_tubes_ini
        </div>
      </div>

      <form id="traversal-form" action="#" onSubmit={handleSubmit} className="flex-1 px-8 py-4 space-y-8 overflow-y-auto">
        {/* URL Input */}
        <div className="space-y-3">
          <label className="flex items-center text-primary/60 font-black gap-2">
            <Globe className="w-4 h-4" />
            Target_URL
          </label>
        </div>
        <div className="relative group">
          <div className="absolute left-0 bottom-0 w-2 h-2 border-l-2 border-b-2 border-primary" />
          <div className="absolute right-0 top-0 w-2 h-2 border-r-2 border-t-2 border-primary" />
          <input
            type="text"
            name="url"
            required={true}
            placeholder="Enter URL"
            className="w-full bg-surface-container/50 border-none p-3 text-primary-dim focus:ring-0 font-mono text-[1em] placeholder:text-primary/20"
            value={formData.url}
            onChange={(e) => setFormData({ ...formData, url: e.target.value })}
          />
        </div>

        {/* CSS Selector Input */}
        <div className="space-y-3">
          <label className="flex items-center text-primary/60 font-black gap-2">
            <Search className="w-4 h-4" />
            CSS_Selector
          </label>
        </div>
        <div className="relative group">
          <div className="absolute top-0 left-0 w-2 h-2 border-t-2 border-l-2 border-primary" />
          <div className="absolute bottom-0 right-0 w-2 h-2 border-b-2 border-r-2 border-primary" />
          <input
            type="text"
            name="selector"
            required={true}
            placeholder="Enter CSS Selector"
            className="w-full bg-surface-container/50 border-none p-3 text-primary-dim focus:ring-0 font-mono text-[1em] placeholder:text-primary/20"
            value={formData.selector}
            onChange={(e) => setFormData({ ...formData, selector: e.target.value })}
          />
        </div>

        {/* Amount Input */}
        <div className="space-y-3">
          <label className="flex items-center text-primary/60 font-black gap-2">
            <Plus className="w-4 h-4" />
            Amount
          </label>
        </div>
        <div className="relative group">
          <div className="absolute left-0 bottom-0 w-2 h-2 border-l-2 border-b-2 border-primary" />
          <div className="absolute right-0 top-0 w-2 h-2 border-r-2 border-t-2 border-primary" />
          <input
            type="number"
            name="amount"
            placeholder="Enter Amount (0 for all)"
            className="w-full bg-surface-container/50 border-none p-3 text-primary-dim focus:ring-0 font-mono text-[1em] placeholder:text-primary/20"
            onChange={(e) => setFormData({ ...formData, amount: parseInt(e.target.value) || 0 })}
          />
        </div>

        {/* Traversal Mode */}
        <div className="space-y-4">
          <label className="flex items-center text-primary/60 font-black gap-2">
            <Zap className="w-4 h-4" />
            Traversal_Mode
          </label>
          <div className="flex items-center justify-between p-3 bg-surface-container/30 border border-outline-variant/20">
            <span className={`transition-colors ${formData.method !== "BFS" ? "text-primary font-black" : "text-on-surface-variant"}`}>DFS</span>
            <button
              type="button"
              onClick={() => setFormData({ ...formData, method: formData.method === "DFS" ? "BFS" : "DFS" })}
              className="relative w-12 h-6 bg-surface-container-highest rounded-full p-1 transition-colors group"
            >
              <motion.div
                animate={{ x: formData.method === "BFS" ? 24 : 0 }}
                className={`w-4 h-4 rounded-full ${formData.method === "BFS" ? "bg-secondary" : "bg-primary"} shadow-[0_0_32px_rgba(100,240,255,0.5)]`}
              />
            </button>
            <span className={`transition-colors ${formData.method === "BFS" ? "text-secondary font-black" : "text-on-surface-variant"}`}>BFS</span>
          </div>
        </div>

      </form>

      {/* Initialize Traversal Button */}
      <div className="p-8 mt-auto border-t border-primary/10 bg-surface/50">
        <motion.button
          type="submit"
          form="traversal-form"
          disabled={isSubmitting}
          whileTap={{ scale: 0.95 }}
          whileHover={{ scaleX: 1.05, scaleY: 1.02, boxShadow: "0 0 2em rgba(100,240,255,0.7)" }}
          className="w-full bg-primary text-on-primary font-black py-4 clip-corner text-xs tracking-[0.5em] flex items-center justify-center gap-2 disabled:opacity-70 disabled:cursor-not-allowed"
        >
          <ChevronRight className="w-4 h-4" />
          {isSubmitting ? "SUBMITTING..." : "OKE_GAS OKE_GAS"}
          <ChevronRight className="w-4 h-4" />
        </motion.button>
        {submitError && (
          <p className="mt-4 text-sm text-error font-mono normal-case tracking-normal">
            {submitError}
          </p>
        )}
      </div>
    </aside>
  );
}

function Hero() {
  const [randomPhrase, setRandomPhrase] = React.useState("");

  React.useEffect(() => {
    const randomPhrases = [
      "Extend Deadlinenya Dong Bang",
      "Tolong ini kudu diapain",
      "Mengapa aku informatika"
    ];

    setRandomPhrase(randomPhrases[Math.floor(Math.random() * randomPhrases.length)]);
  }, []);

  return (
    <section className="relative min-h-screen flex flex-col justify-center overflow-hidden px-16">
      <div className="max-w-4xl z-20">
        <motion.h1
          initial={{ opacity: 0, x: -20 }}
          animate={{ opacity: 1, x: 0 }}
          transition={{ delay: 0.2 }}
          className="text-8xl font-black text-white mb-8 leading-[0.9] tracking-tighter uppercase max-w-full"
        >
          Bang Fariz Tolong <br />
          <span className="text-primary italic drop-shadow-[0_0_32px_rgba(100,240,255,0.5)]">
            {randomPhrase}
          </span>
        </motion.h1>
      </div>

      <div className="absolute right-[-10%] w-4xl h-4xl opacity-30 mix-blend-screen pointer-events-none">
        <motion.img
          animate={{ rotate: 360 }}
          transition={{ duration: 60, repeat: Infinity, ease: "linear" }}
          src="https://picsum.photos/seed/fariz-rifki/1000/1000"
          alt="Holographic visualization"
          className="w-full h-full object-contain filter hue-rotate-180 brightness-150 rounded-full"
          referrerPolicy="no-referrer"
        />
      </div>
    </section>
  )
}