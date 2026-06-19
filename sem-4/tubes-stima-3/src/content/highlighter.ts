import { HIGHLIGHT_CLASS, IMAGE_CLASS, STYLE_ID, TEXT_BLUR_CLASS } from "./constants";
import type { HighlightMetadata, TextHighlightPlan } from "./types.ts";

const TEXT_BLUR_TARGET = [
    "a",
    "button",
    "label",
    "p",
    "li",
    "h1",
    "h2",
    "h3",
    "h4",
    "h5",
    "h6",
    "td",
    "th",
    "blockquote",
    "figcaption",
    "[role='button']",
    "[role='heading']",
    "[role='link']",
].join(", ");

const PAGE_ROOT_TAGS = new Set(["HTML", "BODY"]);

export function ensureHighlightStyles(): void {
    if (document.getElementById(STYLE_ID)) {
        return;
    }

    const style = document.createElement("style");
    style.id = STYLE_ID;
    style.textContent = `
        .${HIGHLIGHT_CLASS} {
            background: rgba(255, 214, 10, 0.68);
            border-bottom: 2px solid #d9480f;
            border-radius: 3px;
            box-decoration-break: clone;
            -webkit-box-decoration-break: clone;
            color: inherit;
            cursor: help;
            padding: 0 0.05em;
        }

        img.${IMAGE_CLASS}[data-judol-action="blur"] {
            cursor: help;
            filter: blur(10px) !important;
            transition: filter 0.2s ease;
        }

        .${TEXT_BLUR_CLASS}[data-judol-action="text-blur"] {
            cursor: help;
            filter: blur(6px) !important;
            transition: filter 0.18s ease;
        }
    `;

    document.head.appendChild(style);
}

export function attachHighlightMetadata(element: HTMLElement, metadata: HighlightMetadata): void {
    element.dataset.keyword = metadata.keyword;
    element.dataset.algorithm = metadata.algorithm;
    element.dataset.occurrences = String(metadata.occurrences);
    element.dataset.executionTime = metadata.executionTime.toFixed(3);
}

export function removeOldHighlights(root: ParentNode = document.body): void {
    const highlights = Array.from(root.querySelectorAll<HTMLElement>(`.${HIGHLIGHT_CLASS}`));
    const parentsToNormalize = new Set<Node>();

    for (const highlight of highlights) {
        const parent = highlight.parentNode;

        if (!parent) {
            continue;
        }

        while (highlight.firstChild) {
            parent.insertBefore(highlight.firstChild, highlight);
        }

        parent.removeChild(highlight);
        parentsToNormalize.add(parent);
    }

    for (const parent of parentsToNormalize) {
        parent.normalize();
    }
}

export function applyTextBlurs(plans: TextHighlightPlan[], root: ParentNode = document.body): void {
    const targetPlans = new Map<HTMLElement, TextHighlightPlan>();

    for (const plan of plans) {
        if (!plan.node.isConnected) {
            continue;
        }

        const target = findTextBlurTarget(plan.node);
        if (!target || !containsNode(root, target)) {
            continue;
        }

        const currentPlan = targetPlans.get(target);
        if (!currentPlan || compareHighlightPlans(plan, currentPlan) < 0) {
            targetPlans.set(target, plan);
        }
    }

    const existingBlurs = new Set(getTextBlurElements(root));

    for (const blurredElement of existingBlurs) {
        if (!targetPlans.has(blurredElement)) {
            clearTextBlurElement(blurredElement);
        }
    }

    for (const [target, plan] of targetPlans) {
        target.classList.add(TEXT_BLUR_CLASS);
        target.dataset.judolAction = "text-blur";
        attachHighlightMetadata(target, plan);
    }
}

export function removeTextBlurs(root: ParentNode = document.body): void {
    const blurredElements = getTextBlurElements(root);

    for (const element of blurredElements) {
        clearTextBlurElement(element);
    }
}

export function normalizeHighlightPlans(plans: TextHighlightPlan[]): TextHighlightPlan[] {
    const grouped = new Map<Text, TextHighlightPlan[]>();

    for (const plan of plans) {
        const plansForNode = grouped.get(plan.node) ?? [];
        plansForNode.push(plan);
        grouped.set(plan.node, plansForNode);
    }

    const selectedPlans: TextHighlightPlan[] = [];

    for (const plansForNode of grouped.values()) {
        const selectedForNode: TextHighlightPlan[] = [];
        const ordered = [...plansForNode].sort(compareHighlightPlans);

        for (const candidate of ordered) {
            if (selectedForNode.some((selected) => rangesOverlap(candidate, selected))) {
                continue;
            }

            selectedForNode.push(candidate);
        }

        selectedPlans.push(...selectedForNode.sort((a, b) => a.startIndex - b.startIndex));
    }

    return selectedPlans;
}

function compareHighlightPlans(left: TextHighlightPlan, right: TextHighlightPlan): number {
    const confidenceDiff = getHighlightConfidenceRank(left) - getHighlightConfidenceRank(right);
    if (confidenceDiff !== 0) {
        return confidenceDiff;
    }

    const lengthDiff = getHighlightLength(right) - getHighlightLength(left);
    if (lengthDiff !== 0) {
        return lengthDiff;
    }

    return left.startIndex - right.startIndex;
}

function getHighlightConfidenceRank(plan: TextHighlightPlan): number {
    if (plan.algorithm === "RegEx") {
        return 0;
    }

    if (plan.algorithm === "Weighted Levenshtein") {
        return 2;
    }

    return 1;
}

function getHighlightLength(plan: TextHighlightPlan): number {
    return plan.endIndex - plan.startIndex;
}

export function applyHighlights(plans: TextHighlightPlan[], root: ParentNode = document.body): void {
    const existingHighlights = new Set(
        Array.from(root.querySelectorAll<HTMLElement>(`.${HIGHLIGHT_CLASS}`)),
    );
    const retainedHighlights = new Set<HTMLElement>();
    const parentsToNormalize = new Set<Node>();
    const grouped = new Map<Text, TextHighlightPlan[]>();

    for (const plan of plans) {
        const plansForNode = grouped.get(plan.node) ?? [];
        plansForNode.push(plan);
        grouped.set(plan.node, plansForNode);
    }

    for (const [node, plansForNode] of grouped) {
        const text = node.nodeValue ?? "";
        const parent = node.parentNode;

        if (!parent || text.length === 0 || !node.isConnected) {
            continue;
        }

        const orderedPlans = plansForNode.sort((a, b) => a.startIndex - b.startIndex);
        const currentHighlight = parent instanceof HTMLElement && parent.classList.contains(HIGHLIGHT_CLASS)
            ? parent
            : null;

        if (currentHighlight) {
            const wholeNodePlan = orderedPlans.length === 1 && orderedPlans[0].startIndex === 0 && orderedPlans[0].endIndex === text.length;
            if (wholeNodePlan) {
                attachHighlightMetadata(currentHighlight, orderedPlans[0]);
                retainedHighlights.add(currentHighlight);
                continue;
            }

            const replacement = buildHighlightFragment(text, orderedPlans);
            const highlightParent = currentHighlight.parentNode;
            if (highlightParent) {
                highlightParent.replaceChild(replacement, currentHighlight);
                parentsToNormalize.add(highlightParent);
            }
            continue;
        }

        const fragment = buildHighlightFragment(text, orderedPlans, retainedHighlights);
        parent.replaceChild(fragment, node);
        parentsToNormalize.add(parent);
    }

    for (const highlight of existingHighlights) {
        if (!retainedHighlights.has(highlight) && highlight.isConnected) {
            const parent = unwrapHighlight(highlight);
            if (parent) {
                parentsToNormalize.add(parent);
            }
        }
    }

    for (const parent of parentsToNormalize) {
        parent.normalize();
    }
}

function buildHighlightFragment(
    text: string,
    plans: TextHighlightPlan[],
    retainedHighlights?: Set<HTMLElement>,
): DocumentFragment {
    const fragment = document.createDocumentFragment();
    let cursor = 0;

    for (const plan of plans) {
        if (plan.startIndex < cursor || plan.endIndex > text.length) {
            continue;
        }

        if (plan.startIndex > cursor) {
            fragment.appendChild(document.createTextNode(text.slice(cursor, plan.startIndex)));
        }

        const highlight = document.createElement("span");
        highlight.className = HIGHLIGHT_CLASS;
        highlight.textContent = text.slice(plan.startIndex, plan.endIndex);
        attachHighlightMetadata(highlight, plan);
        fragment.appendChild(highlight);
        retainedHighlights?.add(highlight);
        cursor = plan.endIndex;
    }

    if (cursor < text.length) {
        fragment.appendChild(document.createTextNode(text.slice(cursor)));
    }

    return fragment;
}

function unwrapHighlight(highlight: HTMLElement): Node | null {
    const parent = highlight.parentNode;
    if (!parent) {
        return null;
    }

    while (highlight.firstChild) {
        parent.insertBefore(highlight.firstChild, highlight);
    }

    parent.removeChild(highlight);
    return parent;
}

function getTextBlurElements(root: ParentNode): HTMLElement[] {
    const elements = Array.from(root.querySelectorAll<HTMLElement>(`.${TEXT_BLUR_CLASS}`));

    if (root instanceof HTMLElement && root.classList.contains(TEXT_BLUR_CLASS)) {
        elements.unshift(root);
    }

    return elements;
}

function clearTextBlurElement(element: HTMLElement): void {
    element.classList.remove(TEXT_BLUR_CLASS);

    if (element.dataset.judolAction === "text-blur") {
        delete element.dataset.judolAction;
        delete element.dataset.keyword;
        delete element.dataset.algorithm;
        delete element.dataset.occurrences;
        delete element.dataset.executionTime;
    }
}

function findTextBlurTarget(node: Text): HTMLElement | null {
    let parent = node.parentElement;

    while (parent?.classList.contains(HIGHLIGHT_CLASS)) {
        parent = parent.parentElement;
    }

    if (!parent || isPageRoot(parent) || isExtensionVisual(parent)) {
        return null;
    }

    const preferredTarget = parent.closest<HTMLElement>(TEXT_BLUR_TARGET);
    if (preferredTarget && !isPageRoot(preferredTarget) && !isExtensionVisual(preferredTarget)) {
        return preferredTarget;
    }

    let current: HTMLElement | null = parent;
    while (current && !isPageRoot(current)) {
        if (!isExtensionVisual(current) && isBlockLikeElement(current)) {
            return current;
        }

        current = current.parentElement;
    }

    return isExtensionVisual(parent) ? null : parent;
}

function isBlockLikeElement(element: HTMLElement): boolean {
    const display = window.getComputedStyle(element).display;
    return display !== "inline" && display !== "contents";
}

function isPageRoot(element: Element): boolean {
    return PAGE_ROOT_TAGS.has(element.tagName);
}

function isExtensionVisual(element: HTMLElement): boolean {
    return (
        element.classList.contains(HIGHLIGHT_CLASS) ||
        element.classList.contains(IMAGE_CLASS)
    );
}

function containsNode(root: ParentNode, node: Node): boolean {
    return root instanceof Node ? root.contains(node) : true;
}

function rangesOverlap(left: TextHighlightPlan, right: TextHighlightPlan): boolean {
    return left.startIndex < right.endIndex && right.startIndex < left.endIndex;
}
