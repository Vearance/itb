import {
  SerializedDOMNode,
  SerializedSearchLogEntry,
  SerializedSelectorResult,
} from "@/types/traversal";

export function parseSessionJSON<T>(value: string | null): T | null {
  if (!value) {
    return null;
  }

  try {
    return JSON.parse(value) as T;
  } catch {
    return null;
  }
}

export function flattenDOMTree(root: SerializedDOMNode): SerializedDOMNode[] {
  const nodes: SerializedDOMNode[] = [];
  const stack: SerializedDOMNode[] = [root];

  while (stack.length > 0) {
    const currentNode = stack.pop();
    if (!currentNode) {
      continue;
    }

    nodes.push(currentNode);

    for (let i = currentNode.children.length - 1; i >= 0; i -= 1) {
      stack.push(currentNode.children[i]);
    }
  }

  return nodes;
}

export function createVisitedNodeSet(
  entries: SerializedSearchLogEntry[],
  currentStep: number
): Set<number> {
  const visitedNodeIds = new Set<number>();

  for (let index = 0; index <= currentStep; index += 1) {
    const entry = entries[index];
    if (!entry) {
      continue;
    }
    visitedNodeIds.add(entry.NodeID);
  }

  return visitedNodeIds;
}

export function createMatchedNodeSet(
  resultMap: Record<string, SerializedSelectorResult>
): Set<number> {
  const matchedNodeIds = new Set<number>();

  for (const result of Object.values(resultMap)) {
    matchedNodeIds.add(result.node.NodeID);
  }

  return matchedNodeIds;
}

export function getNodeLabelPrimary(node: SerializedDOMNode): string {
  let label = node.tag;

  if (node.id) {
    label += `#${node.id}`;
  }

  if (node.classes.length > 0) {
    label += node.classes.map((cls) => `.${cls}`).join("");
  }

  return label;
}
export function getNodeLabelSecondary(node: SerializedDOMNode): string {
  if (node.id) {
    return `#${node.id}`;
  }

  if (node.classes.length > 0) {
    return `.${node.classes[0]}`;
  }

  return `node-${node.nodeID}`;
}
