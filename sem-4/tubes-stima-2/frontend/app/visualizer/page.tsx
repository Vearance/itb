"use client";

import Link from "next/link";
import React from "react";
import { TraversalApiResponse, TraversalRequestPayload } from "@/types/traversal";
import { PlaybackControls } from "./components/PlaybackControls";
import { TraversalLog } from "./components/TraversalLog";
import { TraversalTree } from "./components/TraversalTree";
import {
  createMatchedNodeSet,
  createVisitedNodeSet,
  flattenDOMTree,
  getNodeLabelPrimary,
  parseSessionJSON,
} from "./utils";
import { motion } from "motion/react";

const DEFAULT_PLAYBACK_SPEED_MS = 200;

export default function VisualizerPage() {
  const [requestPayload, setRequestPayload] = React.useState<TraversalRequestPayload | null>(null);
  const [responsePayload, setResponsePayload] = React.useState<TraversalApiResponse | null>(null);
  const [isSessionChecked, setIsSessionChecked] = React.useState(false);
  const [currentStep, setCurrentStep] = React.useState(-1);
  const [isPlaying, setIsPlaying] = React.useState(false);
  const [speedMs, setSpeedMs] = React.useState(DEFAULT_PLAYBACK_SPEED_MS);

  React.useEffect(() => {
    const savedRequest = parseSessionJSON<TraversalRequestPayload>(
      sessionStorage.getItem("traversal:request")
    );
    const savedResponse = parseSessionJSON<TraversalApiResponse>(
      sessionStorage.getItem("traversal:response")
    );

    setRequestPayload(savedRequest);
    setResponsePayload(savedResponse);
    setIsSessionChecked(true);
  }, []);

  const entries = React.useMemo(() => responsePayload?.log.Entries ?? [], [responsePayload]);
  const totalSteps = entries.length;

  React.useEffect(() => {
    if (!responsePayload) {
      return;
    }

    setCurrentStep(totalSteps > 0 ? 0 : -1);
    setIsPlaying(totalSteps > 0);
  }, [responsePayload, totalSteps]);

  React.useEffect(() => {
    if (!isPlaying || !responsePayload || totalSteps === 0) {
      return;
    }

    if (currentStep >= totalSteps - 1) {
      setIsPlaying(false);
      return;
    }

    const timer = window.setTimeout(() => {
      setCurrentStep((previousStep) => Math.min(previousStep + 1, totalSteps - 1));
    }, speedMs);

    return () => {
      window.clearTimeout(timer);
    };
  }, [isPlaying, responsePayload, totalSteps, currentStep, speedMs]);

  const matchedNodeIds = React.useMemo(() => {
    if (!responsePayload) {
      return new Set<number>();
    }
    return createMatchedNodeSet(responsePayload.result);
  }, [responsePayload]);

  const selectedPathsByNodeId = React.useMemo(() => {
    const selectedPathMap = new Map<number, number[]>();

    if (!responsePayload) {
      return selectedPathMap;
    }

    for (const selectorResult of Object.values(responsePayload.result)) {
      selectedPathMap.set(selectorResult.node.NodeID, selectorResult.path);
    }

    return selectedPathMap;
  }, [responsePayload]);

  const allNodes = React.useMemo(() => {
    if (!responsePayload) {
      return [];
    }
    return flattenDOMTree(responsePayload.DOMTree);
  }, [responsePayload]);

  const nodeMap = React.useMemo(() => {
    return new Map(allNodes.map((node) => [node.nodeID, node]));
  }, [allNodes]);

  const visitedNodeIds = React.useMemo(() => {
    return createVisitedNodeSet(entries, currentStep);
  }, [entries, currentStep]);

  const currentEntry = currentStep >= 0 ? entries[currentStep] ?? null : null;
  const currentNode = currentEntry ? nodeMap.get(currentEntry.NodeID) ?? null : null;

  const handlePlayPause = React.useCallback(() => {
    if (totalSteps === 0) {
      return;
    }

    if (currentStep >= totalSteps - 1) {
      setCurrentStep(-1);
      setIsPlaying(true);
      return;
    }

    setIsPlaying((previousValue) => !previousValue);
  }, [currentStep, totalSteps]);

  const handleReset = React.useCallback(() => {
    setCurrentStep(-1);
    setIsPlaying(false);
  }, []);

  const handleStepBack = React.useCallback(() => {
    setIsPlaying(false);
    setCurrentStep((previousStep) => Math.max(previousStep - 1, -1));
  }, []);

  const handleStepForward = React.useCallback(() => {
    if (totalSteps === 0) {
      return;
    }

    setIsPlaying(false);
    setCurrentStep((previousStep) => Math.min(previousStep + 1, totalSteps - 1));
  }, [totalSteps]);

  const handleJumpToStep = React.useCallback((targetStep: number) => {
    setIsPlaying(false);
    setCurrentStep(targetStep);
  }, []);

  const selectedCount = matchedNodeIds.size;
  const traversedCount = Math.max(0, currentStep + 1);

  if (!isSessionChecked) {
    return (
      <div className="min-h-screen bg-background text-white selection:bg-primary selection:text-on-primary px-6 py-12">
        <div className="max-w-3xl mx-auto border border-outline-variant/40 bg-surface-container/40 p-8">
          <h1 className="text-4xl font-black text-white mb-8 leading-[0.9] tracking-tighter uppercase max-w-full">
            <span className="text-primary italic drop-shadow-[0_0_32px_rgba(100,240,255,0.5)]">
              Traversal Dashboard
            </span>
          </h1>
          <p className="mt-4 text-on-surface-variant">
            Loading traversal data...
          </p>
        </div>
      </div>
    );
  }

  if (!responsePayload) {
    return (
      <div className="min-h-screen bg-background text-white selection:bg-primary selection:text-on-primary px-6 py-12">
        <div className="max-w-3xl mx-auto border border-outline-variant/40 bg-surface-container/40 p-8">
          <h1 className="text-4xl font-black text-white mb-8 leading-[0.9] tracking-tighter uppercase max-w-full">
            <span className="text-primary italic drop-shadow-[0_0_32px_rgba(100,240,255,0.5)]">
              Traversal Dashboard
            </span>
          </h1>
          <p className="mt-4 text-on-surface-variant">
            No traversal data found. Submit the form from homepage first.
          </p>
          <Link
            href="/"
            className="inline-flex mt-6 bg-primary text-on-primary font-black px-5 py-3 uppercase tracking-wider"
          >
            Back To Homepage
          </Link>
        </div>
      </div>
    );
  }

  return (
    <div className="h-screen bg-background text-white selection:bg-primary selection:text-on-primary px-4 py-4">
      <div className="max-w-450 mx-auto h-full flex flex-col gap-4">
        <header className="border border-primary/30 bg-surface-container/40 p-4">
          <motion.h1
            initial={{ opacity: 0, y: -20 }}
            animate={{ opacity: 1, y: 0 }}
            transition={{ delay: 0.2 }}
            className="text-xl font-black text-white uppercase tracking-[0.16em]"
          >
            <span className="text-primary italic drop-shadow-[0_0_32px_rgba(100,240,255,0.5)]">
              Traversal Dashboard
            </span>
          </motion.h1>
          <div className="grid grid-cols-1 xl:grid-cols-9 gap-3 mt-4 text-xs font-mono normal-case">
            <div className="bg-surface-container-high/60 px-3 py-2 border border-outline-variant/25 col-span-2">
              <p className="text-on-surface-variant">URL</p>
              <p className="text-primary-dim mt-1 truncate">{requestPayload?.url ?? "404"}</p>
            </div>
            <div className="bg-surface-container-high/60 px-3 py-2 border border-outline-variant/25 col-span-2">
              <p className="text-on-surface-variant">Selector</p>
              <p className="text-primary-dim mt-1 truncate">{responsePayload.log.Selector}</p>
            </div>
            <div className="bg-surface-container-high/60 px-3 py-2 border border-outline-variant/25 col-span-2">
              <p className="text-on-surface-variant">Traversal Type</p>
              <p className="text-primary-dim mt-1">{responsePayload.log.SearchType}</p>
            </div>
            <div className="bg-surface-container-high/60 px-3 py-2 border border-outline-variant/25 col-span-2">
              <p className="text-on-surface-variant">Matched Nodes</p>
              <p className="text-primary-dim mt-1">{selectedCount}</p>
            </div>
            <Link href="/" className="inline-flex items-center justify-center bg-primary text-on-primary font-black px-4 py-2 uppercase tracking-wider col-span-1">
              New Traversal
            </Link>
          </div>
        </header>

        <div className="grid grid-cols-1 xl:grid-cols-6 gap-4 flex-1 min-h-0">
          <section className="xl:col-span-4 min-h-0 flex flex-col gap-4">
            <div>
              <TraversalTree
                root={responsePayload.DOMTree}
                visitedNodeIds={visitedNodeIds}
                matchedNodeIds={matchedNodeIds}
                selectedPathsByNodeId={selectedPathsByNodeId}
                currentEntry={currentEntry}
                defaultViewportHeight={window.innerHeight}
                defaultViewportWidth={window.innerWidth}
              />
            </div>

            <PlaybackControls
              currentStep={currentStep}
              totalSteps={totalSteps}
              speedMs={speedMs}
              isPlaying={isPlaying}
              onPlayPause={handlePlayPause}
              onReset={handleReset}
              onStepBack={handleStepBack}
              onStepForward={handleStepForward}
              onSpeedChange={setSpeedMs}
            />
          </section>

          <aside className="xl:col-span-2 min-h-0 flex flex-col gap-4">
            <section className="border border-outline-variant/35 bg-surface-container/45 p-5 space-y-3">
              <span className="text-primary italic drop-shadow-[0_0_32px_rgba(100,240,255,0.5)]">
                <h2 className="text-lg text-primary font-black uppercase tracking-wider">Current Node</h2>
              </span>
              {currentNode ? (
                <div className="space-y-2 text-sm font-mono normal-case">
                  <p className="text-primary-dim">
                    {getNodeLabelPrimary(currentNode)}
                  </p>
                  <div className="grid grid-cols-2 gap-x-8">
                    <p className="text-on-surface-variant">NodeID: {currentNode.nodeID}</p>
                    <p className="text-on-surface-variant">Depth: {currentNode.depth}</p>
                    <p className="text-on-surface-variant">Traversed Steps: {traversedCount}</p>
                    <p className="text-on-surface-variant">Total Nodes: {allNodes.length}</p>
                  </div>
                </div>
              ) : (
                <p className="text-sm text-on-surface-variant normal-case">
                  Press Play to start traversal playback.
                </p>
              )}
            </section>

            <div className="flex-1 min-h-0">
              <TraversalLog
                entries={entries}
                currentStep={currentStep}
                onJumpToStep={handleJumpToStep}
              />
            </div>
          </aside>
        </div>
      </div>
    </div>
  );
}