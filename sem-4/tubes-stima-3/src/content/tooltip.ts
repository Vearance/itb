import type { HighlightMetadata } from "./types.ts";
import { HIGHLIGHT_CLASS, IMAGE_CLASS, TEXT_BLUR_CLASS, TOOLTIP_CLASS, TOOLTIP_ID, TOOLTIP_ROW_CLASS, TOOLTIP_STYLE_ID } from "./constants.ts";

let listenersAttached = false;
let activeTarget: HTMLElement | null = null;

export function setupTooltipDelegation(): void {
    ensureTooltipStyles();

    if (listenersAttached) {
        return;
    }

    document.addEventListener("mouseover", handlePointerEnter, true);
    document.addEventListener("mouseout", handlePointerLeave, true);
    document.addEventListener("focusin", handleFocusIn, true);
    document.addEventListener("focusout", handleFocusOut, true);
    window.addEventListener("scroll", repositionTooltip, true);
    window.addEventListener("resize", repositionTooltip, true);
    listenersAttached = true;
}

export function removeStaleTooltips(): void {
    const tooltips = Array.from(document.querySelectorAll<HTMLElement>(`.${TOOLTIP_CLASS}`));

    for (const tooltip of tooltips) {
        tooltip.remove();
    }

    activeTarget = null;
}

function handlePointerEnter(event: MouseEvent): void {
    const target = findHighlight(event.target);

    if (target) {
        showTooltip(target);
    }
}

function handlePointerLeave(event: MouseEvent): void {
    if (!activeTarget) {
        return;
    }

    if (event.relatedTarget instanceof Node && activeTarget.contains(event.relatedTarget)) {
        return;
    }

    hideTooltip();
}

function handleFocusIn(event: FocusEvent): void {
    const target = findHighlight(event.target);

    if (target) {
        showTooltip(target);
    }
}

function handleFocusOut(): void {
    hideTooltip();
}

function showTooltip(target: HTMLElement): void {
    activeTarget = target;
    const tooltip = getTooltipElement();
    const metadata = readMetadata(target);

    tooltip.replaceChildren(
        createRow("Keyword", metadata.keyword),
        createRow("Algorithm", metadata.algorithm),
        createRow("Occurrences", String(metadata.occurrences)),
        createRow("Execution", `${metadata.executionTime.toFixed(3)} ms`),
    );

    tooltip.hidden = false;
    repositionTooltip();
}

function hideTooltip(): void {
    const tooltip = document.getElementById(TOOLTIP_ID);

    if (tooltip) {
        tooltip.hidden = true;
    }

    activeTarget = null;
}

function repositionTooltip(): void {
    if (!activeTarget) {
        return;
    }

    const tooltip = document.getElementById(TOOLTIP_ID);

    if (!(tooltip instanceof HTMLElement) || tooltip.hidden) {
        return;
    }

    const targetRect = activeTarget.getBoundingClientRect();
    const tooltipRect = tooltip.getBoundingClientRect();
    const gap = 8;
    const viewportPadding = 8;
    const topCandidate = targetRect.top - tooltipRect.height - gap;
    const top = topCandidate >= viewportPadding ? topCandidate : targetRect.bottom + gap;
    const centeredLeft = targetRect.left + targetRect.width / 2 - tooltipRect.width / 2;
    const maxLeft = window.innerWidth - tooltipRect.width - viewportPadding;
    const left = Math.min(Math.max(viewportPadding, centeredLeft), Math.max(viewportPadding, maxLeft));

    tooltip.style.transform = `translate(${Math.round(left)}px, ${Math.round(top)}px)`;
}

function getTooltipElement(): HTMLElement {
    const existing = document.getElementById(TOOLTIP_ID);

    if (existing instanceof HTMLElement) {
        return existing;
    }

    const tooltip = document.createElement("div");
    tooltip.id = TOOLTIP_ID;
    tooltip.className = TOOLTIP_CLASS;
    tooltip.hidden = true;
    document.body.appendChild(tooltip);
    return tooltip;
}

function createRow(label: string, value: string): HTMLElement {
    const row = document.createElement("div");
    const labelElement = document.createElement("span");
    const valueElement = document.createElement("strong");

    row.className = TOOLTIP_ROW_CLASS;
    labelElement.textContent = label;
    valueElement.textContent = value;
    row.append(labelElement, valueElement);
    return row;
}

function findHighlight(target: EventTarget | null): HTMLElement | null {
    if (!(target instanceof Element)) {
        return null;
    }

    // Text highlights (existing)
    const textHighlight = target.closest<HTMLElement>(`.${HIGHLIGHT_CLASS}`);
    if (textHighlight) {
        return textHighlight;
    }

    // Image highlights — the <img> itself carries the data-* attributes
    const imageHighlight = target.closest<HTMLElement>(`img.${IMAGE_CLASS}`);
    if (imageHighlight) {
        return imageHighlight;
    }

    const textBlur = target.closest<HTMLElement>(`.${TEXT_BLUR_CLASS}`);
    return textBlur;
}

function readMetadata(target: HTMLElement): HighlightMetadata {
    return {
        keyword: target.dataset.keyword ?? "-",
        algorithm: (target.dataset.algorithm ?? "KMP") as HighlightMetadata["algorithm"],
        occurrences: Number(target.dataset.occurrences ?? "0"),
        executionTime: Number(target.dataset.executionTime ?? "0"),
    };
}

function ensureTooltipStyles(): void {
    if (document.getElementById(TOOLTIP_STYLE_ID)) {
        return;
    }

    const style = document.createElement("style");
    style.id = TOOLTIP_STYLE_ID;
    style.textContent = `
        .${TOOLTIP_CLASS} {
            background: #171717;
            border: 1px solid rgba(255, 255, 255, 0.12);
            border-radius: 6px;
            box-shadow: 0 14px 34px rgba(0, 0, 0, 0.28);
            color: #ffffff;
            font: 12px/1.4 system-ui, -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif;
            left: 0;
            max-width: min(280px, calc(100vw - 16px));
            padding: 10px 12px;
            pointer-events: none;
            position: fixed;
            top: 0;
            z-index: 2147483647;
        }

        .${TOOLTIP_CLASS}[hidden] {
            display: none;
        }

        .${TOOLTIP_ROW_CLASS} {
            display: grid;
            gap: 14px;
            grid-template-columns: max-content minmax(0, 1fr);
            min-width: 190px;
        }

        .${TOOLTIP_ROW_CLASS} + .${TOOLTIP_ROW_CLASS} {
            margin-top: 5px;
        }

        .${TOOLTIP_ROW_CLASS} span {
            color: #bdbdbd;
        }

        .${TOOLTIP_ROW_CLASS} strong {
            color: #ffffff;
            font-weight: 650;
            overflow-wrap: anywhere;
            text-align: right;
        }
    `;

    document.head.appendChild(style);
}
