import React from "react";

type PlaybackControlsProps = {
  currentStep: number;
  totalSteps: number;
  speedMs: number;
  isPlaying: boolean;
  onPlayPause: () => void;
  onReset: () => void;
  onStepBack: () => void;
  onStepForward: () => void;
  onSpeedChange: (nextSpeed: number) => void;
};

const SPEED_OPTIONS = [
  { label: "x2.0", value: 100 },
  { label: "x1.5", value: 200 },
  { label: "x1.0", value: 400 },
  { label: "x0.5", value: 600 },
  { label: "x0.25", value: 800 },
];

export function PlaybackControls({
  currentStep,
  totalSteps,
  speedMs,
  isPlaying,
  onPlayPause,
  onReset,
  onStepBack,
  onStepForward,
  onSpeedChange,
}: PlaybackControlsProps) {
  const traversedCount = Math.max(0, currentStep + 1);
  const progressPercent =
    totalSteps > 0 ? Math.min(100, (traversedCount / totalSteps) * 100) : 0;

  return (
    <section className="border border-primary/25 bg-surface-container/50 p-5 space-y-4">
      <div>
        <span className="text-primary italic drop-shadow-[0_0_32px_rgba(100,240,255,0.5)]">
          <h2 className="text-lg text-primary font-black uppercase tracking-wider">
            Playback
          </h2>
        </span>
        <p className="text-xs text-on-surface-variant mt-1 font-mono normal-case">
          Step {Math.max(0, currentStep + 1)} of {totalSteps}
        </p>
      </div>

      <div className="h-2 bg-surface-container-highest rounded-full overflow-hidden">
        <div
          className="h-full bg-primary transition-[width] duration-150"
          style={{ width: `${progressPercent}%` }}
        />
      </div>

      <div className="grid grid-cols-4 gap-2">
        <button
          type="button"
          onClick={onStepBack}
          className="px-3 py-2 bg-surface-container-highest hover:bg-surface-container text-xs font-black uppercase tracking-wider"
        >
          Back
        </button>
        <button
          type="button"
          onClick={onPlayPause}
          disabled={totalSteps === 0}
          className="px-3 py-2 bg-primary text-on-primary disabled:opacity-50 text-xs font-black uppercase tracking-wider"
        >
          {isPlaying ? "Pause" : "Play"}
        </button>
        <button
          type="button"
          onClick={onStepForward}
          className="px-3 py-2 bg-surface-container-highest hover:bg-surface-container text-xs font-black uppercase tracking-wider"
        >
          Next
        </button>
        <button
          type="button"
          onClick={onReset}
          className="px-3 py-2 bg-surface-container-highest hover:bg-surface-container text-xs font-black uppercase tracking-wider"
        >
          Reset
        </button>
      </div>

      <div className="space-y-2">
        <p className="text-xs font-black uppercase tracking-wider text-primary/80">
          Speed
        </p>
        <div className="flex flex-wrap gap-2">
          {SPEED_OPTIONS.map((option) => {
            const isActive = option.value === speedMs;

            return (
              <button
                key={option.value}
                type="button"
                onClick={() => onSpeedChange(option.value)}
                className={`px-2 py-1 text-xs font-black uppercase tracking-wider border ${isActive
                  ? "border-primary bg-primary text-on-primary"
                  : "border-outline-variant bg-surface-container-highest text-on-surface-variant"
                  }`}
              >
                {option.label}
              </button>
            );
          })}
        </div>
      </div>
    </section>
  );
}
