import keywordFile from "../../keywords/keyword.txt?raw";

export const KEYWORDS = parseKeywords(keywordFile);

// parse keyword dan clean up whitespace
function parseKeywords(raw: string): string[] {
    return raw
        .split(/\r?\n/u)
        .map((line) => line.trim())
        .filter((line) => line.length > 0);
}
