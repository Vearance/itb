import type { MatchResult } from "../content/types.ts";

const BASE = 31; // small prime number used as the polynomial radix
const MOD  = 1_000_000_007; // large prime number used to keep hash values in safe-integer range and minimise hash collisions

// maps a character to a non-zero numeric value.
function charCode(c: string): number {
    const code = c.charCodeAt(0);
    if (code >= 97 && code <= 122){
        return code - 96;
    }
    if (code >= 48 && code <= 57){
        return code - 48 + 27;
    }
    return code + 37;
}


// computes polynomial rolling hash of string s
// bdsk fungsi H(s) = s[0]*BASE^(n-1) + s[1]*BASE^(n-2) + … + s[n-1]*BASE^0   (mod MOD)
function computeHash(s: string): number {
    let h = 0;
    for (let i = 0; i < s.length; i++) {
        h = (h * BASE + charCode(s[i])) % MOD;
    }
    return h;
}

//pre-compute BASE^(m-1) mod MOD ; m = pattern length
function computeLeftmostWeight(m: number): number {
    let result = 1;
    for (let i = 0; i < m - 1; i++) {
        result = (result * BASE) % MOD;
    }
    return result;
}


// Rabin-Karp search algorithm
export function rabinKarp(
    text: string,
    pattern: string,
    caseSensitive = false,
): MatchResult {
    const startTime = performance.now();

    if (pattern.length === 0 || text.length === 0 || pattern.length > text.length) {
        return {
            matched: false, matchIndexes: [], matchLengths: [],
            comparisonCount: 0, executionTime: performance.now() - startTime, algorithm: "RabinKarp",
        };
    }

    let t = text;
    let p = pattern;
    if (!caseSensitive) {
        t = text.toLowerCase();
        p = pattern.toLowerCase();
    }

    const n = t.length;
    const m = p.length;
    const patternHash = computeHash(p);
    const highPow = computeLeftmostWeight(m);
    let windowHash = computeHash(t.slice(0, m));
    const matchIndexes: number[] = [];
    const matchLengths: number[] = [];
    let comparisonCount = 0;

    //sliding-window loop
    for (let i = 0; i <= n - m; i++) {
        comparisonCount++;

        if (windowHash === patternHash) { // check if hash match, then verify char-by-char
            let isMatch = true;
            for (let k = 0; k < m; k++) {
                comparisonCount++;
                if (t[i + k] !== p[k]) {
                    isMatch = false;
                    break;
                }
            }
            if (isMatch) {
                matchIndexes.push(i);
                matchLengths.push(m);
            }
        }

        // roll the hash for the next window
        if (i < n - m) {
            // rumus: H(next) = ((H(curr) - outgoing) * BASE + incoming) % MOD
            const outgoing = (charCode(t[i]) * highPow) % MOD;
            windowHash = ((windowHash - outgoing + MOD) * BASE + charCode(t[i + m])) % MOD;
        }
    }

    return {
        matched: matchIndexes.length > 0, matchIndexes, matchLengths,
        comparisonCount, executionTime: performance.now() - startTime, algorithm: "RabinKarp",
    };
}


//runs rabin-karp over multiple keywords against one text
export function rabinKarpMulti(text: string, keywords: string[]): Map<string, MatchResult> {
    const results = new Map<string, MatchResult>();
    for (const keyword of keywords) {
        const trimmed = keyword.trim();
        if (trimmed.length === 0){
            continue;
        }
        results.set(trimmed, rabinKarp(text, trimmed));
    }
    return results;
}