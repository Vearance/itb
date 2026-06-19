import type { MatchResult } from "../content/types.ts";

// membangun failure table
function buildFailureTable(pattern: string): number[] {
  const m = pattern.length;
  const failure: number[] = new Array(m).fill(0);
  let prefixLen = 0;
  let i = 1;

  while (i < m) {
    if (pattern[i] === pattern[prefixLen]) {
      prefixLen++;
      failure[i] = prefixLen;
      i++;
    } else {
      if (prefixLen !== 0) {
        prefixLen = failure[prefixLen - 1];
      } else {
        failure[i] = 0; // gagal mencocokkan karakter
        i++;
      }
    }
  }

  return failure;
}

// mencari seluruh kemunculan pattern dalam teks
export function kmp(text: string, pattern: string, caseSensitive = false): MatchResult {
  const startTime = performance.now();

  if (pattern.length === 0 || text.length === 0 || pattern.length > text.length) {
    return {
      matched: false, matchIndexes: [], comparisonCount: 0,
      executionTime: performance.now() - startTime, algorithm: "KMP"
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
  const failure = buildFailureTable(p);
  const matchIndexes: number[] = [];
  let comparisonCount = 0;
  let i = 0;
  let j = 0;

  while (i < n) {
    comparisonCount++;

    if (t[i] === p[j]) {
      i++;
      j++;

      if (j === m) {
        matchIndexes.push(i - m);
        j = failure[m - 1];
      }
    } else {
      if (j !== 0) { // mismatch stlh bbrp karakter cocok
        j = failure[j - 1];
      } else { // mismatch di awal pattern
        i++;
      }
    }
  }

  return {
    matched: matchIndexes.length > 0, matchIndexes, comparisonCount,
    executionTime: performance.now() - startTime, algorithm: "KMP"
  };

}

