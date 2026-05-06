import React from "react";
import { SerializedSearchLogEntry } from "@/types/traversal";

type TraversalLogProps = {
  entries: SerializedSearchLogEntry[];
  currentStep: number;
  onJumpToStep: (stepIndex: number) => void;
};

export function TraversalLog({
  entries,
  currentStep,
  onJumpToStep,
}: TraversalLogProps) {
  const activeRowRef = React.useRef<HTMLButtonElement | null>(null);

  const activeStep = React.useMemo(() => {
    if (entries.length === 0) {
      return -1;
    }

    if (currentStep < 0) {
      return 0;
    }

    return Math.min(currentStep, entries.length - 1);
  }, [entries.length, currentStep]);

  const renderedEntries = React.useMemo(() => {
    return entries.map((entry, absoluteIndex) => ({
      entry,
      absoluteIndex,
    }));
  }, [entries]);

  React.useEffect(() => {
    if (!activeRowRef.current) {
      return;
    }

    activeRowRef.current.scrollIntoView({
      behavior: "smooth",
      block: "nearest",
    });
  }, [activeStep]);

  return (
    <section className="h-full border border-outline-variant/40 bg-surface-container/40 p-5 flex flex-col min-h-0">
      <span className="text-primary italic drop-shadow-[0_0_32px_rgba(100,240,255,0.5)]">
        <h2 className="text-lg text-primary font-black uppercase tracking-wider">
          Traversal Log
        </h2>
      </span>
      
      <p className="text-xs text-on-surface-variant mt-1 font-mono normal-case">
        Full traversal timeline loaded. Click any row to jump forward or backward.
      </p>

      <div className="mt-4 overflow-auto flex-1 min-h-0">
        {renderedEntries.length === 0 ? (
          <p className="text-sm text-on-surface-variant normal-case">
            Start playback to stream traversal entries.
          </p>
        ) : (
          <ul className="space-y-1">
            {renderedEntries.map(({ entry, absoluteIndex }) => {
              const isCurrent = absoluteIndex === activeStep;

              return (
                <li key={`${absoluteIndex}-${entry.NodeID}`}>
                  <button
                    ref={isCurrent ? activeRowRef : null}
                    type="button"
                    onClick={() => onJumpToStep(absoluteIndex)}
                    className={`w-full text-left p-2 border transition-colors ${isCurrent
                      ? "border-primary bg-primary/10"
                      : "border-outline-variant/25 bg-surface-container-highest hover:border-primary/40"
                      }`}
                  >
                    <div className="flex items-center justify-between gap-2 font-mono text-xs">
                      <span className="text-primary-dim">Step: #{absoluteIndex + 1}</span>
                      <span className="text-on-surface-variant">ID: {entry.NodeID}</span>
                      <span className="text-on-surface-variant">Depth {entry.Depth}</span>
                    </div>
                    <div className="mt-1 flex gap-2 text-xs uppercase tracking-wider font-black">
                      {entry.CandidateNode && (
                        <span className="px-2 py-0.5 bg-amber-200/20 text-amber-200">
                          Candidate
                        </span>
                      )}
                      {entry.SelectedNode && (
                        <span className="px-2 py-0.5 bg-emerald-300/20 text-emerald-300">
                          Selected
                        </span>
                      )}
                      {!entry.CandidateNode && !entry.SelectedNode && (
                        <span className="px-2 py-0.5 bg-surface-container text-on-surface-variant">
                          Traversed
                        </span>
                      )}
                    </div>
                  </button>
                </li>
              );
            })}
          </ul>
        )}
      </div>
    </section>
  );
}
