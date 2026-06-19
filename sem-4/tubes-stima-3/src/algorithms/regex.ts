import type { CanonicalText, MatcherOutput, TextMatch } from "../content/types";

const ENDS_WITH_NUMS =
    /(^|[^a-z0-9])([a-z][a-z\d]*[a-z]\d{2,3})(?![a-z0-9])/gu;

export function regexMatcher(text: string, canonical: CanonicalText): MatcherOutput {
    const startedAt = performance.now(); // performance.now() is more accurate for measuring short durations than Date.now()
    const matches: TextMatch[] = [];
    ENDS_WITH_NUMS.lastIndex = 0;

    try {
        let result = ENDS_WITH_NUMS.exec(canonical.value);

        while (result !== null) {
            const prefix = result[1] ?? "";
            const detected = result[2] ?? "";
            const start = result.index + prefix.length;
            const end = start + detected.length;
            const startMap = canonical.map[start];
            const endMap = canonical.map[end - 1];

            if (startMap !== undefined && endMap !== undefined) {
                matches.push({
                    start: startMap.start,
                    end: endMap.end,
                    keyword: text.slice(startMap.start, endMap.end),
                    canonicalKeyword: detected,
                    algorithm: "RegEx",
                });
            }

            if (ENDS_WITH_NUMS.lastIndex === result.index) {
                ENDS_WITH_NUMS.lastIndex += 1;
            }

            result = ENDS_WITH_NUMS.exec(canonical.value);
        }
    } finally {
        ENDS_WITH_NUMS.lastIndex = 0;
    }

    return {
        matches,
        executionMs: performance.now() - startedAt,
    };
}
