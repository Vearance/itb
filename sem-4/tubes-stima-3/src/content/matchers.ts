import { kmp } from "../algorithms/kmp.ts";
import { bm } from "../algorithms/bm.ts";
import { rabinKarp } from "../algorithms/rabinKarp.ts";
import { PatternMatcher } from "../algorithms/ahoCorasick.ts";
import { fuzzyMatching } from "../algorithms/weightedLevenshteinDistance.ts";
import { KEYWORDS } from "./keywords.ts";
import { normalizeForKey } from "./unicode.ts";
import type { CanonicalText, MatchResult, SourceMapEntry } from "./types.ts";

const CANONICAL_KEYWORDS = [...new Set(KEYWORDS.map(normalizeForKey))];
const acTrie = new PatternMatcher(CANONICAL_KEYWORDS);

// pencarian utama untuk satu keyword pada satu teks
// menerima pre-computed canonical text dan pre-computed A-C results 
export function matchKeyword(text: string, keyword: string, canonical: CanonicalText, acMatches: Map<string, number[]>, acComparisonCount: number, acElapsedMs: number): MatchResult[] {
    const canonicalKeyword = normalizeForKey(keyword);

    // Exact pattern matching
    const kmpResult = mapCanonicalResult(kmp(canonical.value, canonicalKeyword, true), canonical.map, canonicalKeyword.length);
    const bmResult = mapCanonicalResult(bm(canonical.value, canonicalKeyword, true), canonical.map, canonicalKeyword.length);
    const rkResult = mapCanonicalResult(rabinKarp(canonical.value, canonicalKeyword, true), canonical.map, canonicalKeyword.length);

    // Aho-Corasick
    const acIndexes = acMatches.get(canonicalKeyword) ?? [];
    const acResult = mapCanonicalResult({
        matched: acIndexes.length > 0,
        matchIndexes: acIndexes,
        matchLengths: acIndexes.map(() => canonicalKeyword.length),
        comparisonCount: acComparisonCount,
        executionTime: acElapsedMs,
        algorithm: "AhoCorasick",
    }, canonical.map, canonicalKeyword.length);

    const exactMatched = kmpResult.matched || bmResult.matched || rkResult.matched || acResult.matched;
    const wlResult: MatchResult = exactMatched
        ? {
            matched: false,
            matchIndexes: [],
            matchLengths: [],
            comparisonCount: 0,
            executionTime: 0,
            algorithm: "Weighted Levenshtein",
        }
        : fuzzyMatching(keyword, text);

    return [kmpResult, bmResult, rkResult, acResult, wlResult];
}

// menjalankan Aho-Corasick sekali untuk seluruh keyword pada canonical text
export function runAhoCorasickOnce(canonicalText: string): { matches: Map<string, number[]>; comparisonCount: number; elapsedMs: number } {
    const start = performance.now();
    const { matches, comparisonCount } = acTrie.processText(canonicalText);
    return { matches, comparisonCount, elapsedMs: performance.now() - start };
}

function mapCanonicalResult(result: MatchResult, sourceMap: SourceMapEntry[], fallbackLength: number): MatchResult {
    if (!result.matched) {
        return result;
    }

    const matchIndexes: number[] = [];
    const matchLengths: number[] = [];

    for (let i = 0; i < result.matchIndexes.length; i++) {
        const canonicalStart = result.matchIndexes[i];
        const canonicalLength = result.matchLengths?.[i] ?? fallbackLength;
        const originalRange = mapCanonicalRange(sourceMap, canonicalStart, canonicalLength);

        if (originalRange === null) {
            continue;
        }

        matchIndexes.push(originalRange.start);
        matchLengths.push(originalRange.end - originalRange.start);
    }

    return {
        ...result,
        matched: matchIndexes.length > 0,
        matchIndexes,
        matchLengths,
    };
}

function mapCanonicalRange(sourceMap: SourceMapEntry[], start: number, length: number): SourceMapEntry | null {
    const first = sourceMap[start];
    const last = sourceMap[start + length - 1];

    if (first === undefined || last === undefined) {
        return null;
    }

    return {
        start: first.start,
        end: last.end,
    };
}
