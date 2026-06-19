import type { MatchResult } from "../content/types.ts";

// rightmost occurrence of every character in the pattern
function buildLastOccurenceTable(pattern: string) : Map<string, number> {
    const tab = new Map<string, number>();

    for (let i = 0; i < pattern.length; i++) {
        tab.set(pattern[i], i);
    }
    return tab;
}

// Case 1 – Mismatch become match 
// Case 2 – Pattern move past the mismatch character
export function bm(text: string, pattern: string, caseSensitive = false): MatchResult {
    const startTime = performance.now();

    if (pattern.length === 0 || text.length === 0 || pattern.length > text.length) {
        return {
            matched: false, matchIndexes: [], comparisonCount: 0,
            executionTime: performance.now() - startTime, algorithm: "BM"
            };
    }
    let t = caseSensitive ? text : text.toLowerCase();
    let p =  caseSensitive ? pattern : pattern.toLowerCase();
    let comparisonCount = 0;
    const tab = buildLastOccurenceTable(p);
    // boyer-moore
    const matchIndexes: number[] = [];

    let m = p.length;
    let n = t.length
    let skip = 1;
    for (let i = 0; i <= n - m; i+= skip) {
        skip = 0;
        // cek di pattern
        for (let j = m - 1; j >= 0; j--) {
            // karakter paling ujung kanan di pattern
            // != karakter posisi i + ituannya pattern
            let tPost : string = t.charAt(i + j);

            comparisonCount++;

            if (p.charAt(j) != tPost) {
                let shift = j - (tab.get(tPost) ?? -1); // ga ada di pattern
                skip = Math.max(1, shift);
                break;
            }
        }
        if (skip === 0) {
            matchIndexes.push(i);
            skip = 1;
        }
        
    }
    return {
        matched: matchIndexes.length > 0, matchIndexes, comparisonCount,
        executionTime: performance.now() - startTime, algorithm: "BM"
    };
}