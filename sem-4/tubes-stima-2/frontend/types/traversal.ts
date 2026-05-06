export type TraversalMethod = "DFS" | "BFS";

export type TraversalRequestPayload = {
  url: string;
  selector: string;
  amount: number;
  type: TraversalMethod;
};

export type SerializedDOMNode = {
  nodeID: number;
  tag: string;
  id: string;
  classes: string[];
  attributes: Record<string, string>;
  content: string;
  depth: number;
  children: SerializedDOMNode[];
};

export type SerializedResultNode = {
  NodeID: number;
  Tag: string;
  ID: string;
  Classes: string[];
  Attributes: Record<string, string>;
  Content: string;
  Depth: number;
};

export type SerializedSelectorResult = {
  node: SerializedResultNode;
  path: number[];
};

export type SerializedSearchLog = {
  Selector: string;
  SearchType: TraversalMethod;
  Entries: SerializedSearchLogEntry[];
};

export type SerializedSearchLogEntry = {
  NodeID: number;
  Depth: number;
  CandidateNode: boolean;
  SelectedNode: boolean;
};

export type TraversalApiResponse = {
  DOMTree: SerializedDOMNode;
  result: Record<string, SerializedSelectorResult>;
  log: SerializedSearchLog;
};

export type TraversalApiErrorResponse = {
  field?: string;
  error?: string;
};
