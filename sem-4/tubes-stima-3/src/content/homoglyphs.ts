import homoglyphChars from "../../keywords/chars.txt?raw";

// taken from https://github.com/codebox/homoglyph/blob/master/raw_data/chars.txt

const ASCII_LETTER = /^[A-Za-z]$/u;
const ASCII_LOWERCASE_LETTER = /^[a-z]$/u;
const ASCII_DIGIT_OR_SYMBOL = /^[\x00-\x7F]$/u;

const MANUAL_OVERRIDES: Record<string, string> = {
    // The codebox group "1Iil|" is intentionally broad. Keep common i-like and l-like Unicode characters separated so words like slot stay readable.
    "\u0131": "i",
    "\u0269": "i",
    "\u026a": "i",
    "\u0399": "i",
    "\u03b9": "i",
    "\u0406": "i",
    "\u0456": "i",
    "\u04c0": "i",
    "\u0196": "l",
    "\u01c0": "l",
    "\u04cf": "l",
    "\u2223": "l",
};

export const HOMOGLYPH_SKELETON = {
    ...buildHomoglyphMap(homoglyphChars),
    ...MANUAL_OVERRIDES,
};

function buildHomoglyphMap(rawGroups: string): Record<string, string> {
    const map: Record<string, string> = {};

    rawGroups
        .split(/\r?\n/u)
        .map((line) => Array.from(line.trim()))
        .filter((chars) => chars.length > 0)
        .forEach((chars) => {
            const anchor = chooseAnchor(chars);

            if (anchor === undefined) {
                return;
            }

            chars.forEach((char) => {
                const normalizedChar = char.normalize("NFKD").toLowerCase();

                if (shouldSkipMapping(normalizedChar, anchor)) {
                    return;
                }

                map[normalizedChar] = anchor;
            });
        });

    return map;
}

function chooseAnchor(chars: string[]): string | undefined {
    const firstChar = chars[0];

    if (firstChar !== undefined && ASCII_LETTER.test(firstChar)) {
        return firstChar.toLowerCase();
    }

    const anchors = new Set<string>();

    chars.forEach((char) => {
        const lowered = char.toLowerCase();

        if (ASCII_LOWERCASE_LETTER.test(lowered)) {
            anchors.add(lowered);
        }
    });

    if (anchors.size === 1) {
        return [...anchors][0];
    }

    if (anchors.has("o")) {
        return "o";
    }

    if (anchors.has("i")) {
        return "i";
    }

    return undefined;
}

function shouldSkipMapping(normalizedChar: string, anchor: string): boolean {
    if (normalizedChar === anchor) {
        return true;
    }

    if (ASCII_DIGIT_OR_SYMBOL.test(normalizedChar)) {
        return true;
    }

    return normalizedChar.length !== 1;
}
