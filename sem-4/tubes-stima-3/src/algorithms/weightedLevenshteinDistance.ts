import type { MatchResult } from "../content/types.ts";

const SUBSTITUTION_PAIRS: Array<readonly [string, string, number]> = [
    ["0", "o", 0.1],
    ["1", "l", 0.1],
    ["1", "i", 0.1],
    ["3", "e", 0.2],
    ["4", "a", 0.2],
    ["5", "s", 0.2],
    ["7", "t", 0.25],
    ["8", "b", 0.3],
    ["9", "g", 0.1],
    ["@", "a", 0.1],
    ["$", "s", 0.1],
    ["!", "i", 0.2],
    ["!", "l", 0.2],
];

const MIN_FUZZY_KEYWORD_LENGTH = 4;
const MAX_FUZZY_TEXT_LENGTH = 160;
const FUZZY_THRESHOLD = 0.7;
const WHITESPACE = /\s/u;

type SubstitutionCostTable = Record<string, Record<string, number>>;

interface DistanceResult {
    distance: number;
    comparisonCount: number;
}

interface DistanceWorkspace {
    previous: Float64Array;
    current: Float64Array;
}

const substitutionCosts: SubstitutionCostTable = Object.create(null) as SubstitutionCostTable;

for (const [left, right, cost] of SUBSTITUTION_PAIRS) {
    setSubstitutionCost(left, right, cost);
    setSubstitutionCost(right, left, cost);
}

function setSubstitutionCost(from: string, to: string, cost: number): void {
    const costsFromChar = substitutionCosts[from] ?? (substitutionCosts[from] = Object.create(null) as Record<string, number>);
    costsFromChar[to] = cost;
}

function getSubstitutionCost(left: string, right: string): number {
    if (left === right) {
        return 0;
    }

    return substitutionCosts[left]?.[right] ?? 1;
}

function createEmptyResult(startTime: number): MatchResult {
    return {
        matched: false,
        matchIndexes: [],
        matchLengths: [],
        comparisonCount: 0,
        executionTime: performance.now() - startTime,
        algorithm: "Weighted Levenshtein",
    };
}

function createDistanceWorkspace(keywordLength: number): DistanceWorkspace {
    return {
        previous: new Float64Array(keywordLength + 1),
        current: new Float64Array(keywordLength + 1),
    };
}

function hasWhitespaceEdge(text: string, startIndex: number, length: number): boolean {
    return WHITESPACE.test(text[startIndex] ?? "") || WHITESPACE.test(text[startIndex + length - 1] ?? "");
}

function weightedLevenshteinDistanceAt(
    text: string,
    startIndex: number,
    candidateLength: number,
    keyword: string,
    maxDistance: number,
    workspace: DistanceWorkspace,
): DistanceResult {
    const keywordLength = keyword.length;
    let comparisonCount = 0;

    let previous = workspace.previous;
    let current = workspace.current;

    previous[0] = 0;

    for (let j = 1; j <= keywordLength; j++) {
        previous[j] = j;
    }

    for (let i = 1; i <= candidateLength; i++) {
        current[0] = i;

        let rowMinimum = current[0];
        const candidateChar = text[startIndex + i - 1];

        for (let j = 1; j <= keywordLength; j++) {
            comparisonCount++;

            const deletionCost = previous[j] + 1;
            const insertionCost = current[j - 1] + 1;
            const substitutionCost = previous[j - 1] + getSubstitutionCost(candidateChar, keyword[j - 1]);
            let bestCost = deletionCost < insertionCost ? deletionCost : insertionCost;

            if (substitutionCost < bestCost) {
                bestCost = substitutionCost;
            }

            current[j] = bestCost;
            if (bestCost < rowMinimum) {
                rowMinimum = bestCost;
            }
        }

        if (rowMinimum > maxDistance) {
            return { distance: rowMinimum, comparisonCount };
        }

        const swap = previous;
        previous = current;
        current = swap;
    }

    return { distance: previous[keywordLength], comparisonCount };
}

export function fuzzyMatching(keyword: string, text: string): MatchResult {
    const startTime = performance.now();

    if (
        keyword.length < MIN_FUZZY_KEYWORD_LENGTH
        || text.length === 0
        || keyword.length > text.length
        || text.length > MAX_FUZZY_TEXT_LENGTH
    ) {
        return createEmptyResult(startTime);
    }

    const matchIndexes: number[] = [];
    const matchLengths: number[] = [];
    let comparisonCount = 0;
    const keywordLength = keyword.length;
    const lowerText = text.toLowerCase();
    const lowerKeyword = keyword.toLowerCase();
    const maxDistance = (1 - FUZZY_THRESHOLD) * keywordLength;
    const workspace = createDistanceWorkspace(keywordLength);

    for (let i = 0; i <= lowerText.length - keywordLength; i++) {
        if (hasWhitespaceEdge(lowerText, i, keywordLength)) {
            continue;
        }

        const result = weightedLevenshteinDistanceAt(lowerText, i, keywordLength, lowerKeyword, maxDistance, workspace);

        comparisonCount += result.comparisonCount;

        if (result.distance <= maxDistance) {
            matchIndexes.push(i);
            matchLengths.push(keywordLength);
        }
    }

    return {
        matched: matchIndexes.length > 0,
        matchIndexes,
        matchLengths,
        comparisonCount,
        executionTime: performance.now() - startTime,
        algorithm: "Weighted Levenshtein",
    };
}