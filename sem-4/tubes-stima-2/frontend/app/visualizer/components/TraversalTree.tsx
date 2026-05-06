import React from "react";
import { hierarchy, select, tree, zoom, zoomIdentity } from "d3";
import { SerializedDOMNode, SerializedSearchLogEntry } from "@/types/traversal";
import { getNodeLabelSecondary } from "../utils";

type TraversalTreeProps = {
  root: SerializedDOMNode;
  visitedNodeIds: Set<number>;
  matchedNodeIds: Set<number>;
  selectedPathsByNodeId: Map<number, number[]>;
  currentEntry: SerializedSearchLogEntry | null;
  defaultViewportHeight?: number;
  defaultViewportWidth?: number;
};

type NodeVisualState = {
  fill: string;
  stroke: string;
  text: string;
  opacity: number;
};

const ZOOM_SCALE_EXTENT: [number, number] = [0.3, 3.2];

function resolveVisualState(
  nodeId: number,
  visitedNodeIds: Set<number>,
  matchedNodeIds: Set<number>,
  currentEntry: SerializedSearchLogEntry | null
): NodeVisualState {
  const isVisited = visitedNodeIds.has(nodeId);
  const isMatched = matchedNodeIds.has(nodeId);
  const isCurrent = currentEntry?.NodeID === nodeId;
  const isCurrentCandidate = isCurrent && currentEntry?.CandidateNode;
  const isCurrentSelected = isCurrent && currentEntry?.SelectedNode;

  if (isCurrentSelected) {
    return {
      fill: "color-mix(in srgb, var(--color-primary) 24%, transparent)",
      stroke: "var(--color-primary)",
      text: "var(--color-primary)",
      opacity: 1,
    };
  }

  if (isCurrentCandidate) {
    return {
      fill: "color-mix(in srgb, var(--color-tertiary) 24%, transparent)",
      stroke: "var(--color-tertiary)",
      text: "var(--color-secondary)",
      opacity: 1,
    };
  }

  if (isMatched && isVisited) {
    return {
      fill: "color-mix(in srgb, var(--color-secondary) 18%, transparent)",
      stroke: "var(--color-secondary)",
      text: "var(--color-secondary)",
      opacity: 1,
    };
  }


  if (isVisited) {
    return {
      fill: "color-mix(in srgb, var(--color-primary-dim) 18%, transparent)",
      stroke: "var(--color-primary-dim)",
      text: "var(--color-primary)",
      opacity: 1,
    };
  }

  return {
    fill: "color-mix(in srgb, var(--color-surface-container-highest) 35%, transparent)",
    stroke: "var(--color-outline-variant)",
    text: "var(--color-on-surface-variant)",
    opacity: 0.4,
  };
}

export function TraversalTree({
  root,
  visitedNodeIds,
  matchedNodeIds,
  selectedPathsByNodeId,
  currentEntry,
  defaultViewportHeight = 640,
  defaultViewportWidth = 980
}: TraversalTreeProps) {
  const containerRef = React.useRef<HTMLDivElement | null>(null);
  const svgRef = React.useRef<SVGSVGElement | null>(null);
  const zoomBehaviorRef = React.useRef<ReturnType<typeof zoom<SVGSVGElement, unknown>> | null>(null);
  const zoomTransformRef = React.useRef(zoomIdentity);
  const hasInitializedTransformRef = React.useRef(false);

  const [viewportWidth, setViewportWidth] = React.useState(defaultViewportWidth);
  const [viewportHeight, setViewportHeight] = React.useState(defaultViewportHeight);
  const [zoomTransform, setZoomTransform] = React.useState(zoomIdentity);
  const [manualFocusedNodeId, setManualFocusedNodeId] = React.useState<number | null>(null);

  const layout = React.useMemo(() => {
    const rootHierarchy = hierarchy(root, (node) => node.children);
    const treeLayout = tree<SerializedDOMNode>().nodeSize([54, 170]);
    const laidOutRoot = treeLayout(rootHierarchy);

    const nodes = laidOutRoot.descendants();
    const links = laidOutRoot.links();

    const minX = Math.min(...nodes.map((node) => node.x));
    const maxX = Math.max(...nodes.map((node) => node.x));
    const maxY = Math.max(...nodes.map((node) => node.y));

    const paddingX = 110;
    const paddingY = 90;

    const contentWidth = Math.max(defaultViewportWidth, maxY + paddingX * 2);
    const contentHeight = Math.max(defaultViewportHeight, maxX - minX + paddingY * 2);
    const nodePositions = new Map<number, { x: number; y: number }>();
    const parentByNodeId = new Map<number, number | null>();

    for (const node of nodes) {
      const x = node.y + paddingX;
      const y = node.x - minX + paddingY;
      nodePositions.set(node.data.nodeID, { x, y });
      parentByNodeId.set(
        node.data.nodeID,
        node.parent ? node.parent.data.nodeID : null
      );
    }

    return {
      minX,
      paddingX,
      paddingY,
      contentWidth,
      contentHeight,
      nodePositions,
      parentByNodeId,
      nodes,
      links,
    };
  }, [root, defaultViewportWidth, defaultViewportHeight]);

  const focusedPath = React.useMemo(() => {
    if (manualFocusedNodeId === null) {
      return [] as number[];
    }

    const selectedPath = selectedPathsByNodeId.get(manualFocusedNodeId);
    if (selectedPath && selectedPath.length > 0) {
      return selectedPath;
    }

    const derivedPath: number[] = [];
    const seen = new Set<number>();
    let currentNodeId: number | null | undefined = manualFocusedNodeId;

    while (currentNodeId !== null && currentNodeId !== undefined && !seen.has(currentNodeId)) {
      seen.add(currentNodeId);
      derivedPath.push(currentNodeId);
      currentNodeId = layout.parentByNodeId.get(currentNodeId);
    }

    derivedPath.reverse();
    return derivedPath;
  }, [layout.parentByNodeId, manualFocusedNodeId, selectedPathsByNodeId]);

  const focusedPathNodeIds = React.useMemo(() => {
    return new Set(focusedPath);
  }, [focusedPath]);

  const focusedPathEdgeKeys = React.useMemo(() => {
    const edgeKeys = new Set<string>();

    if (manualFocusedNodeId === null) {
      return edgeKeys;
    }

    if (focusedPath.length < 2) {
      return edgeKeys;
    }

    for (let index = 0; index < focusedPath.length - 1; index += 1) {
      edgeKeys.add(`${focusedPath[index]}-${focusedPath[index + 1]}`);
    }

    return edgeKeys;
  }, [focusedPath, manualFocusedNodeId]);

  const isManualFocusedNodeSelected =
    manualFocusedNodeId !== null && selectedPathsByNodeId.has(manualFocusedNodeId);

  const focusNode = React.useCallback((nodeId: number, scale: number, durationMs = 160) => {
    const zoomBehavior = zoomBehaviorRef.current;
    const svgElement = svgRef.current;
    const targetPosition = layout.nodePositions.get(nodeId);

    if (!zoomBehavior || !svgElement || !targetPosition) {
      return;
    }

    const clampedScale = Math.max(
      ZOOM_SCALE_EXTENT[0],
      Math.min(ZOOM_SCALE_EXTENT[1], scale)
    );

    const targetTransform = zoomIdentity
      .translate(
        viewportWidth / 2 - targetPosition.x * clampedScale,
        viewportHeight / 2 - targetPosition.y * clampedScale
      )
      .scale(clampedScale);

    const svgSelection = select(svgElement);
    svgSelection.interrupt();

    if (durationMs <= 0) {
      svgSelection.call(zoomBehavior.transform, targetTransform);
      return;
    }

    svgSelection
      .transition()
      .duration(durationMs)
      .call(zoomBehavior.transform, targetTransform);
  }, [layout.nodePositions, viewportHeight, viewportWidth]);

  const handleNodeDoubleClick = React.useCallback((event: React.MouseEvent<SVGGElement>, nodeId: number) => {
    event.preventDefault();
    event.stopPropagation();

    setManualFocusedNodeId(nodeId);
    focusNode(nodeId, Math.max(zoomTransformRef.current.k, 1.05), 190);
  }, [focusNode]);

  const handleCanvasDoubleClick = React.useCallback((event: React.MouseEvent<SVGSVGElement>) => {
    if (event.target !== event.currentTarget) {
      return;
    }

    setManualFocusedNodeId(null);

    if (currentEntry) {
      focusNode(currentEntry.NodeID, zoomTransformRef.current.k, 190);
    }
  }, [currentEntry, focusNode]);

  React.useEffect(() => {
    const container = containerRef.current;
    if (!container) {
      return;
    }

    const updateWidth = () => {
      const nextWidth = Math.max(
        defaultViewportWidth,
        Math.floor(container.getBoundingClientRect().width)
      );
      setViewportWidth(nextWidth);
    };

    updateWidth();

    if (typeof ResizeObserver === "undefined") {
      return;
    }

    const observer = new ResizeObserver(() => updateWidth());
    observer.observe(container);

    return () => {
      observer.disconnect();
    };
  }, []);

  React.useEffect(() => {
    const svgElement = svgRef.current;
    if (!svgElement) {
      return;
    }

    const svgSelection = select(svgElement);
    const zoomBehavior = zoom<SVGSVGElement, unknown>()
      .scaleExtent(ZOOM_SCALE_EXTENT)
      .on("zoom", (event) => {
        zoomTransformRef.current = event.transform;
        setZoomTransform(event.transform);
      });

    zoomBehaviorRef.current = zoomBehavior;
    svgSelection.call(zoomBehavior);
    svgSelection.on("dblclick.zoom", null);

    return () => {
      svgSelection.on(".zoom", null);
    };
  }, []);

  React.useEffect(() => {
    hasInitializedTransformRef.current = false;
    setManualFocusedNodeId(null);
  }, [root]);

  React.useEffect(() => {
    const zoomBehavior = zoomBehaviorRef.current;
    const svgElement = svgRef.current;
    const rootPosition = layout.nodePositions.get(root.nodeID);

    if (!zoomBehavior || !svgElement || !rootPosition || hasInitializedTransformRef.current) {
      return;
    }

    focusNode(root.nodeID, 0.75, 0);
    hasInitializedTransformRef.current = true;
  }, [focusNode, layout.nodePositions, root.nodeID]);

  React.useEffect(() => {
    if (!currentEntry || manualFocusedNodeId !== null) {
      return;
    }

    focusNode(currentEntry.NodeID, zoomTransformRef.current.k, 160);
  }, [currentEntry, focusNode, manualFocusedNodeId]);

  return (
    <div className="border border-primary/20 bg-surface-container/30 p-4">
      <div className="mb-4 flex items-center justify-between gap-3">
        <div>
          <span className="text-primary italic drop-shadow-[0_0_32px_rgba(100,240,255,0.5)]">
            <h2 className="text-lg text-primary font-black uppercase tracking-wider">
              DOM Tree
            </h2>
          </span>

          <p className="text-xs text-on-surface-variant mt-1 font-mono normal-case">
            Drag to pan, scroll to zoom, double-click node to focus, double-click canvas to resume auto-follow.
          </p>
        </div>
        <div className="text-xs font-mono text-on-surface-variant normal-case">
          Current Node: {currentEntry ? currentEntry.NodeID : "-"} | Focus: {manualFocusedNodeId ?? "auto"}
        </div>
      </div>

      <div
        ref={containerRef}
        className="w-full overflow-hidden rounded-lg border border-outline-variant/30 bg-background/80"
        style={{ height: `${viewportHeight}px` }}
      >
        <svg
          ref={svgRef}
          width={viewportWidth}
          height={viewportHeight}
          className="touch-none cursor-grab active:cursor-grabbing"
          onDoubleClick={handleCanvasDoubleClick}
        >
          <g transform={zoomTransform.toString()}>
            {layout.links.map((link) => {
              const sourceId = link.source.data.nodeID;
              const targetId = link.target.data.nodeID;
              const sourceX = link.source.y + layout.paddingX;
              const sourceY = link.source.x - layout.minX + layout.paddingY;
              const targetX = link.target.y + layout.paddingX;
              const targetY = link.target.x - layout.minX + layout.paddingY;

              const midX = (sourceX + targetX) / 2;
              const path = `M ${sourceX} ${sourceY} C ${midX} ${sourceY}, ${midX} ${targetY}, ${targetX} ${targetY}`;

              const isPathLink = focusedPathEdgeKeys.has(`${sourceId}-${targetId}`);
              const isLinkVisible =
                isPathLink || visitedNodeIds.has(targetId) || currentEntry?.NodeID === targetId;

              return (
                <path
                  key={`${sourceId}-${targetId}`}
                  d={path}
                  fill="none"
                  stroke={
                    isPathLink
                      ? "#6effa1"
                      : isLinkVisible
                        ? "#00e2ed"
                        : "#333232"
                  }
                  strokeOpacity={isPathLink ? 0.95 : isLinkVisible ? 0.75 : 0.2}
                  strokeWidth={isPathLink ? 3.2 : isLinkVisible ? 2.2 : 1.4}
                  style={{ transition: "all 200ms ease" }}
                />
              );
            })}

            {layout.nodes.map((node) => {
              const nodeId = node.data.nodeID;
              const x = node.y + layout.paddingX;
              const y = node.x - layout.minX + layout.paddingY;
              const visualState = resolveVisualState(
                nodeId,
                visitedNodeIds,
                matchedNodeIds,
                currentEntry
              );

              const isPathNode = focusedPathNodeIds.has(nodeId);
              const isManualFocusNode = manualFocusedNodeId === nodeId;
              const circleFill =
                isManualFocusNode && isManualFocusedNodeSelected
                  ? "rgba(110, 255, 161, 0.32)"
                  : isManualFocusNode
                    ? "rgba(255, 223, 116, 0.3)"
                    : isPathNode
                      ? "rgba(110, 255, 161, 0.22)"
                      : visualState.fill;
              const circleStroke =
                isManualFocusNode
                  ? "#ffffff"
                  : isPathNode
                    ? "#6effa1"
                    : visualState.stroke;
              const textColor = isPathNode || isManualFocusNode ? "#e6ffe9" : visualState.text;
              const nodeOpacity = isPathNode || isManualFocusNode ? 1 : visualState.opacity;

              return (
                <g
                  key={nodeId}
                  transform={`translate(${x},${y})`}
                  opacity={nodeOpacity}
                  onDoubleClick={(event) => handleNodeDoubleClick(event, nodeId)}
                  className="cursor-pointer"
                  style={{ transition: "all 200ms ease" }}
                >
                  <circle
                    r={12}
                    fill={circleFill}
                    stroke={circleStroke}
                    strokeWidth={isManualFocusNode ? 3.2 : 2.2}
                  />

                  <text
                    x={20}
                    y={-4}
                    fill={textColor}
                    fontFamily="var(--font-mono)"
                    fontSize={11}
                    fontWeight={700}
                  >
                    {node.data.tag}
                  </text>
                  <text
                    x={20}
                    y={10}
                    fill="#bcbcbc"
                    fontFamily="var(--font-mono)"
                    fontSize={10}
                  >
                    {getNodeLabelSecondary(node.data)}
                  </text>
                </g>
              );
            })}
          </g>
        </svg>
      </div>

      <div className="mt-3 text-xs text-on-surface-variant font-mono normal-case">
        Canvas: {viewportWidth} x {viewportHeight.toFixed(0)} | Scale: {zoomTransform.k.toFixed(2)}x | Tree Size: {Math.round(layout.contentWidth)} x {Math.round(layout.contentHeight)} | Mode: {manualFocusedNodeId === null ? "Auto-follow" : isManualFocusedNodeSelected ? "Manual focus (path highlighted)" : "Manual focus"}
      </div>
    </div>
  );
}
