export const ALGORITHMS = [
    "KMP",
    "BM",
    "RegEx",
    "Weighted Levenshtein",
    "AhoCorasick",
    "RabinKarp",
] as const;

export type AlgorithmTypes = (typeof ALGORITHMS)[number];

export interface AlgorithmStats {
    matches: number;
    executionMs: number;
    todo: boolean;
}

// Hasil pencarian satu pattern pada satu teks
export interface MatchResult {
    matched: boolean;
    matchIndexes: number[]; //idx awal setiap kemunculan pattern di teks
    matchLengths?: number[];
    comparisonCount: number; // jmlh opr perbandingan karakter
    executionTime: number;
    algorithm: AlgorithmTypes;
}

export interface RecordAlgorithmOptions {
    countCost?: boolean;
}

//hasil pencarian smw keyword dari keyword.txt, map dari keyword sbg key ke hasil matchingnya sbg value
export type MultiKeywordResult = Map<string, MatchResult>;

export interface SourceMapEntry {
    start: number;
    end: number;
} // some unicode chars could be expanded into more than one char in the canonical form, so we need to track start/end pos

export interface CanonicalText {
    value: string;  // the normalized text used for matching
    map: SourceMapEntry[]; // maps each character in 'value' back to its position in the original text
}

export interface TextMatch {
    start: number;
    end: number;
    keyword: string;
    canonicalKeyword: string;
    algorithm: AlgorithmTypes;
}

export interface MatcherOutput {
    matches: TextMatch[];
    executionMs: number;
}

export interface HighlightMetadata {
    keyword: string;
    algorithm: AlgorithmTypes;
    occurrences: number;
    executionTime: number;
}

export interface TextHighlightPlan extends HighlightMetadata {
    node: Text;
    startIndex: number;
    endIndex: number;
    text: string;
}

export interface AlgorithmStatistic {
    algorithm: AlgorithmTypes;
    matches: number;
    executionTime: number;
    comparisonCount: number;
}

export interface KeywordFrequency {
    keyword: string;
    occurrences: number;
}

export interface ScanStatistics {
    scanId: string;
    scannedAt: number;
    pageUrl: string;
    pageTitle: string;
    totalKeywordsFound: number;
    totalMatches: number;
    algorithms: Record<AlgorithmTypes, AlgorithmStatistic>;
    keywordFrequency: KeywordFrequency[];
}

export interface ExtensionSettings {
    blurTextEnabled: boolean;
}

export interface ExtensionMessage {
    type: "JUDOL_SCAN_STATS_UPDATED" | "JUDOL_RESCAN_REQUEST";
    payload?: ScanStatistics;
}

export interface ChartDatum {
    label: string;
    value: number;
    color?: string;
}

export interface ChromeStorageChange<T = unknown> {
    oldValue?: T;
    newValue?: T;
}

export interface ChromeStorageArea {
    get(keys: string | string[] | Record<string, unknown> | null, callback: (items: Record<string, unknown>) => void): void;
    set(items: Record<string, unknown>, callback?: () => void): void;
}

export interface ChromeStorageChangedEvent {
    addListener(callback: (changes: Record<string, ChromeStorageChange>, areaName: string) => void): void;
}

export interface ChromeRuntimeMessageEvent {
    addListener(
        callback: (message: ExtensionMessage, sender: unknown, sendResponse: (response?: unknown) => void) => boolean | void,
    ): void;
}

export interface ChromeRuntimeLike {
    sendMessage?(message: ExtensionMessage, callback?: (response?: unknown) => void): Promise<unknown> | void;
    onMessage?: ChromeRuntimeMessageEvent;
    lastError?: { message?: string };
}

export interface ChromeTabsLike {
    query?(queryInfo: Record<string, unknown>, callback: (tabs: Array<{ id?: number; url?: string }>) => void): void;
    sendMessage?(tabId: number, message: ExtensionMessage, callback?: (response?: unknown) => void): void;
}

export interface ChromeLike {
    storage?: {
        local?: ChromeStorageArea;
        onChanged?: ChromeStorageChangedEvent;
    };
    runtime?: ChromeRuntimeLike;
    tabs?: ChromeTabsLike;
}
