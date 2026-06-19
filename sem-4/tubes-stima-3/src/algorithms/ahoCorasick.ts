import type { MatchResult } from "../content/types.ts";

class TrieNode {

    children: Map<string, TrieNode>;

    // waktu mismatch
    failureLink: TrieNode | null;

    // pattern yang berakhir di node ini
    outputs: string[];

    constructor() {
        this.children = new Map<string, TrieNode>();
        this.failureLink = null;
        this.outputs = [];
    }
}

export class PatternMatcher {
    private root: TrieNode;

    constructor(patterns: string[]) {
        this.root = new TrieNode();

        // bangun trie dulu
        this.buildTrie(patterns);

        // nambahin failure links menggunakan BFS
        this.buildFailureLinks();
    }

    private buildTrie(patterns: string[]): void {
        for (const pattern of patterns) {
            let curr = this.root;

            for (const char of pattern) {
                // Jika edge belum ada, buat node baru
                if (!curr.children.has(char)) {
                    curr.children.set(char, new TrieNode());
                }
                // move next
                curr = curr.children.get(char)!;
            }

            curr.outputs.push(pattern);
        }
    }


    // suffix terpanjang
    // yang valid di trie.
    private buildFailureLinks(): void {
        const queue: TrieNode[] = [];

        this.root.failureLink = this.root;


        for (const child of this.root.children.values()) {
            child.failureLink = this.root;
            queue.push(child);
        }

        while (queue.length > 0) {
            const current = queue.shift()!;

            for (const [char, child] of current.children) {

                let failNode = current.failureLink!;


                // Selama belum sampai root tidak ada edge char maka terus fallback
                while ( failNode !== this.root && !failNode.children.has(char)) {
                    failNode = failNode.failureLink!;
                }

                // kalo ada transition, node-nya jadikan fail link
                // guard: jangan self-loop (child menunjuk ke dirinya sendiri)
                if (failNode.children.has(char) && failNode.children.get(char) !== child) {
                    child.failureLink = failNode.children.get(char)!;
                } else {
                    child.failureLink = this.root;
                }

                child.outputs.push(...child.failureLink.outputs);
                queue.push(child);
            }
        }
    }

    public processText( text: string): { matches: Map<string, number[]>, comparisonCount: number} {
        const matches = new Map<string, number[]>();
        let comparisonCount = 0;
        let current = this.root;

        for (let i = 0; i < text.length; i++) {
            const char = text[i];
            comparisonCount++;

            // kalo ga ada transisi, fallback
            while (current !== this.root && !current.children.has(char)) {
                current = current.failureLink!;
            }

            // kalo ada edge, ke next node
            if (current.children.has(char)) {
                current = current.children.get(char)!;
            }

            for (const pattern of current.outputs) {
                // Hitung index awal pattern
                const startIndex = i - pattern.length + 1;

                if (!matches.has(pattern)) {
                    matches.set(pattern, []);
                }
                matches.get(pattern)!.push(startIndex);
            }
        }

        return {
            matches,
            comparisonCount
        };
    }
}

export function ahoCorasick(text: string, keywords: string[]): Map<string, MatchResult> {
    const startTime = performance.now();

    const resultMap = new Map<string, MatchResult>();

    if (keywords.length === 0 || text.length === 0) {
        return resultMap;
    }

    const matcher = new PatternMatcher(keywords);

    const { matches, comparisonCount } = matcher.processText(text);

    for (const keyword of keywords) {
        const indexes = matches.get(keyword) || [];

        const lengths = indexes.map(() => keyword.length);

        resultMap.set(keyword, { matched: indexes.length > 0,
            matchIndexes: indexes, matchLengths: lengths,
            comparisonCount, executionTime: performance.now() - startTime,
            algorithm: "AhoCorasick"
        });
    }

    return resultMap;
}