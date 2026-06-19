import { HIGHLIGHT_CLASS, IMAGE_CLASS, TEXT_BLUR_CLASS, TOOLTIP_CLASS, TOOLTIP_ID } from "./constants";

const SKIPPED_TAGS = new Set([
    "SCRIPT",
    "STYLE",
    "NOSCRIPT",
    "TEXTAREA",
    "INPUT",
    "SELECT",
    "OPTION",
    "CODE",
    "PRE",
    "SVG",
    "CANVAS",
]);

const VISIBLE_TEXT = /\S/u;

export function collectTextNodes(root: HTMLElement): Text[] {
    const nodes: Text[] = [];
    const walker = document.createTreeWalker(root, NodeFilter.SHOW_TEXT, {
        acceptNode(node) {
            if (node instanceof Text && !shouldSkipTextNode(node)) {
                return NodeFilter.FILTER_ACCEPT;
            }

            return NodeFilter.FILTER_REJECT;
        },
    });

    let current = walker.nextNode();

    while (current !== null) {
        if (current instanceof Text) {
            nodes.push(current);
        }

        current = walker.nextNode();
    }

    return nodes;
}

// cek apakah mutasi page perlu rescan (bukan mutasi dari highlight/tooltip/image)
export function isRelevantMutation(mutation: MutationRecord): boolean {
    if (mutation.type === "attributes") {
        return mutation.target instanceof HTMLImageElement || !isArtifactNode(mutation.target);
    }

    if (mutation.type === "characterData") {
        return !isArtifactNode(mutation.target);
    }

    const changedNodes = [...Array.from(mutation.addedNodes), ...Array.from(mutation.removedNodes)];
    if (changedNodes.length === 0) {
        return !isArtifactNode(mutation.target);
    }

    return changedNodes.some((node) => !isArtifactNode(node));
}

function shouldSkipTextNode(node: Text): boolean {
    if (!VISIBLE_TEXT.test(node.data)) {
        return true;
    }

    const parent = node.parentElement;

    if (parent === null) {
        return true;
    }

    if (SKIPPED_TAGS.has(parent.tagName)) {
        return true;
    }

    if (parent.id === TOOLTIP_ID) {
        return true;
    }

    return parent.closest(`#${TOOLTIP_ID}`) !== null;
}

// cek apakah node DOM adalah bagian dari elemen visual buatan extension
function isArtifactNode(node: Node): boolean {
    const element = node instanceof Element ? node : node.parentElement;
    if (!element) {
        return false;
    }

    return (
        element.classList.contains(HIGHLIGHT_CLASS) ||
        element.classList.contains(IMAGE_CLASS) ||
        element.classList.contains(TEXT_BLUR_CLASS) ||
        element.classList.contains(TOOLTIP_CLASS) ||
        element.closest(`.${HIGHLIGHT_CLASS}, .${IMAGE_CLASS}, .${TEXT_BLUR_CLASS}, .${TOOLTIP_CLASS}`) !== null
    );
}
