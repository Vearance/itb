import { HOMOGLYPH_SKELETON } from "./homoglyphs";
import type { CanonicalText, SourceMapEntry } from "./types";

// \p{M} = Unicode Mark
const COMBINING_MARK = /\p{M}/u;

export function toCanonicalText(source: string): CanonicalText {
    let value = "";
    const map: SourceMapEntry[] = [];
    let sourceIndex = 0;

    while (sourceIndex < source.length) {
        // ambil code point dari source, bisa > 1 unit UTF-16
        const codePoint = source.codePointAt(sourceIndex);
        if (codePoint === undefined) {
            break;
        }

        // ubah code point ke string karakter original
        const original = String.fromCodePoint(codePoint);
        const sourceEnd = sourceIndex + original.length;
        const expanded = normalizeChar(original);
        let expandedIndex = 0;

        while (expandedIndex < expanded.length) {
            const expandedCodePoint = expanded.codePointAt(expandedIndex);

            if (expandedCodePoint === undefined) {
                break;
            }

            const normalized = String.fromCodePoint(expandedCodePoint);
            value += normalized;

            map.push({ start: sourceIndex, end: sourceEnd });
            
            expandedIndex += normalized.length;
        }
        sourceIndex = sourceEnd;
    }

    return { value, map };
}

export function normalizeForKey(value: string): string {
    // ambil value canonical; buang mapping
    return toCanonicalText(value).value;
}

function normalizeChar(character: string): string {
    const decomposed = character.normalize("NFKD");
    let normalized = "";
    let index = 0;

    while (index < decomposed.length) {
        const codePoint = decomposed.codePointAt(index);

        if (codePoint === undefined) {
            break;
        }

        const piece = String.fromCodePoint(codePoint);
        index += piece.length;

        if (COMBINING_MARK.test(piece)) {
            continue;
        }

        const lowered = piece.toLowerCase();

        const mapped = HOMOGLYPH_SKELETON[lowered];
        if (mapped !== undefined) {
            normalized += mapped;
        } else {
            normalized += lowered;
        }
    }

    return normalized;
}
