import { collectTextNodes, isRelevantMutation } from "./domScanner.ts";
import { regexMatcher } from "../algorithms/regex.ts";
import { IMAGE_CLASS } from "./constants.ts";
import { applyHighlights, applyTextBlurs, ensureHighlightStyles, normalizeHighlightPlans, removeOldHighlights, removeTextBlurs } from "./highlighter.ts";
import { KEYWORDS } from "./keywords.ts";
import { matchKeyword, runAhoCorasickOnce } from "./matchers.ts";
import { addRescanListener, addSettingsListener, DEFAULT_EXTENSION_SETTINGS, loadExtensionSettings, publishScanStatistics } from "./storage.ts";
import { applyFinalHighlightMetadata, createEmptyScanStatistics, finalizeScanStatistics, recordAlgorithmExecution, recordAlgorithmMatches, recordAlgorithmResult } from "./statistics.ts";
import { removeStaleTooltips, setupTooltipDelegation } from "./tooltip.ts";
import { collectImages, getImageScanKey, markImageProcessed } from "./imageDetection.ts";
import { getOCRExtractor, processOCRResult, registerOCRExtractor, restoreImage } from "./imageReplacement.ts";
import { extractText, terminateOCR } from "./ocrEngine.ts";
import { toCanonicalText } from "./unicode.ts";
import type { MatchResult, ScanStatistics, TextHighlightPlan, TextMatch } from "./types.ts";

const rescanDelayMs = 350;
const maxOcrCacheEntries = 128;
let observer: MutationObserver | null = null;
let rescanTimer: number | undefined;
let scanSequence = 0;
let scanRunning = false;
let scanPending = false;
let imageScanRunning = false;
let pendingImageScan: ImageScanJob | null = null;
let isApplyingHighlights = false;
let extensionSettings = DEFAULT_EXTENSION_SETTINGS;
const ocrTextCache = new Map<string, Promise<string>>();

interface ImageScanJob {
    stats: ScanStatistics;
    keywordAlgorithmTimes: Map<string, number>;
    expectedSeq: number;
}

startContentScript();

// Menentukan waktu tunggu inisialisasi awal saat dokumen telah siap diproses
function startContentScript(): void {
    if (document.readyState === "loading") {
        document.addEventListener("DOMContentLoaded", initialize, { once: true });
        return;
    }

    initialize();
}

function initialize(): void {
    if (!document.body) {
        window.setTimeout(initialize, 50);
        return;
    }
    ensureHighlightStyles();
    setupTooltipDelegation();
    addRescanListener(() => scheduleRescan(0));
    registerOCRExtractor(extractText);
    window.addEventListener("pagehide", () => void terminateOCR(), { once: true });
    removeStaleTooltips();
    removeOldHighlights();
    removeTextBlurs();
    addSettingsListener((settings) => {
        extensionSettings = settings;
        scheduleRescan(0);
    });
    startObserver();
    scheduleRescan(0);
    loadExtensionSettings((settings) => {
        extensionSettings = settings;
        scheduleRescan(0);
    });
}

// Inisialisasi MutationObserver untuk mendeteksi perubahan dinamis pada struktur halaman web
function startObserver(): void {
    observer?.disconnect();
    observer = new MutationObserver((mutations) => {
        if (isApplyingHighlights) {
            return;
        }

        if (mutations.some(isRelevantMutation)) {
            scheduleRescan(rescanDelayMs);
        }
    });

    observer.observe(document.body, {
        attributeFilter: ["src", "srcset", "sizes", "data-src", "data-srcset"],
        attributes: true,
        characterData: true,
        childList: true,
        subtree: true,
    });
}

// Menunda proses pemindaian agar tidak terjadi rescan berulang-ulang dalam waktu singkat
function scheduleRescan(delay: number): void {
    scanPending = true;
    scanSequence++;
    window.clearTimeout(rescanTimer);
    rescanTimer = window.setTimeout(() => {
        void drainScanQueue();
    }, delay);
}

// yields to the browser so it can repaint and handle user input between batches
function yieldToBrowser(): Promise<void> {
    return new Promise((resolve) => setTimeout(resolve, 0));
}

// OPT 5: time-based yielding — yield if batch took > YIELD_BUDGET_MS
const YIELD_BUDGET_MS = 12;

// Menjalankan proses pemindaian utama secara async dengan batched yielding
async function drainScanQueue(): Promise<void> {
    if (scanRunning) {
        return;
    }

    scanRunning = true;
    try {
        while (scanPending) {
            scanPending = false;
            await runScan(scanSequence);
        }
    } finally {
        scanRunning = false;
    }
}

async function runScan(currentSeq: number): Promise<void> {
    if (!document.body) {
        return;
    }

    let stats: ReturnType<typeof createEmptyScanStatistics>;
    let keywordAlgorithmTimes: Map<string, number>;

    try {
        stats = createEmptyScanStatistics(`${Date.now()}-${currentSeq}`);
        keywordAlgorithmTimes = new Map<string, number>();

        const textNodes = collectTextNodes(document.body);
        const rawPlans = await buildHighlightPlansAsync(textNodes, stats, keywordAlgorithmTimes, currentSeq);

        // abort if a newer scan was triggered while we were yielding
        if (scanSequence !== currentSeq) {
            return;
        }

        const plans = normalizeHighlightPlans(rawPlans);

        applyFinalHighlightMetadata(plans, keywordAlgorithmTimes);
        finalizeScanStatistics(stats, plans);

        withObserverPaused(() => {
            removeStaleTooltips();
            updateTextBlurState(plans);
            applyHighlights(plans);
        });

        publishScanStatistics(stats);
    } catch {
        // Runtime pages can mutate aggressively while scanning; failed scans are retried by the next observer tick.
        return;
    }

    // run async OCR image scan with sequence guard
    scheduleImageScan({ stats: stats!, keywordAlgorithmTimes: keywordAlgorithmTimes!, expectedSeq: currentSeq });
}

function updateTextBlurState(plans: TextHighlightPlan[]): void {
    if (!extensionSettings.blurTextEnabled) {
        removeTextBlurs();
        return;
    }

    try {
        applyTextBlurs(plans);
    } catch {
        removeTextBlurs();
    }
}

function withObserverPaused(action: () => void): void {
    const shouldRestartObserver = observer !== null;
    isApplyingHighlights = true;
    observer?.disconnect();

    try {
        action();
    } finally {
        isApplyingHighlights = false;
        if (shouldRestartObserver && document.body) {
            startObserver();
        }
    }
}

// Menganalisis text nodes secara async dalam batch, yield ke browser di antara batch
// OPT 1: Aho-Corasick dijalankan sekali per text node, bukan sekali per keyword
// OPT 2: canonical text di-compute sekali per text node
// OPT 5: time-based yielding alih-alih batch-count fixed
async function buildHighlightPlansAsync(
    textNodes: Text[],
    stats: ReturnType<typeof createEmptyScanStatistics>,
    keywordAlgorithmTimes: Map<string, number>,
    expectedSeq: number,
): Promise<TextHighlightPlan[]> {
    const plans: TextHighlightPlan[] = [];
    let batchStartTime = performance.now();

    for (let n = 0; n < textNodes.length; n++) {
        // abort if a newer scan was triggered
        if (scanSequence !== expectedSeq) {
            return plans;
        }

        const node = textNodes[n];
        const text = node.nodeValue ?? "";

        // OPT 2: compute canonical text sekali per text node
        const canonical = toCanonicalText(text);

        // OPT 1: jalankan Aho-Corasick sekali untuk semua keyword pada text node ini
        const acResult = runAhoCorasickOnce(canonical.value);
        recordAlgorithmExecution(stats, "AhoCorasick", acResult.elapsedMs, acResult.comparisonCount);

        const regexOutput = regexMatcher(text, canonical);
        recordAlgorithmExecution(stats, "RegEx", regexOutput.executionMs, text.length);
        recordAlgorithmMatches(stats, "RegEx", regexOutput.matches.length);
        appendRegexHighlightPlans(plans, node, regexOutput.matches, regexOutput.executionMs);

        for (const keyword of KEYWORDS) {
            const results = matchKeyword(text, keyword, canonical, acResult.matches, acResult.comparisonCount, acResult.elapsedMs);
            for (const result of results) {
                recordAlgorithmResult(stats, result, keyword, keywordAlgorithmTimes, {
                    countCost: !isSharedTextNodeAlgorithm(result),
                });
            }

            appendHighlightPlans(plans, node, text, keyword, results);

            const elapsed = performance.now() - batchStartTime;
            if (elapsed >= YIELD_BUDGET_MS) {
                await yieldToBrowser();
                batchStartTime = performance.now();

                if (scanSequence !== expectedSeq) {
                    return plans;
                }
            }
        }

        // OPT 5: time-based yielding — yield if we've exceeded the budget
        const elapsed = performance.now() - batchStartTime;
        if (elapsed >= YIELD_BUDGET_MS && n < textNodes.length - 1) {
            await yieldToBrowser();
            batchStartTime = performance.now();

            if (scanSequence !== expectedSeq) {
                return plans;
            }
        }
    }

    return plans;
}

function appendHighlightPlans(
    plans: TextHighlightPlan[],
    node: Text,
    text: string,
    keyword: string,
    results: MatchResult[],
): void {
    for (const result of results) {
        if (!result.matched) {
            continue;
        }

        for (let i = 0; i < result.matchIndexes.length; i++) {
            const startIndex = result.matchIndexes[i];
            const matchLength = result.matchLengths?.[i] ?? keyword.length;
            const endIndex = Math.min(text.length, startIndex + matchLength);

            if (startIndex < 0 || endIndex <= startIndex) {
                continue;
            }

            plans.push({
                node,
                startIndex,
                endIndex,
                text: text.slice(startIndex, endIndex),
                keyword,
                algorithm: result.algorithm,
                occurrences: 0,
                executionTime: result.executionTime,
            });
        }
    }
}

function isSharedTextNodeAlgorithm(result: MatchResult): boolean {
    return result.algorithm === "AhoCorasick";
}

function appendRegexHighlightPlans(
    plans: TextHighlightPlan[],
    node: Text,
    matches: TextMatch[],
    executionTime: number,
): void {
    for (const match of matches) {
        plans.push({
            node,
            startIndex: match.start,
            endIndex: match.end,
            text: match.keyword,
            keyword: match.keyword,
            algorithm: "RegEx",
            occurrences: 0,
            executionTime,
        });
    }
}

// Menjalankan pemindaian gambar secara async menggunakan OCR extractor yang telah terdaftar
async function runImageScan(stats: ScanStatistics, keywordAlgorithmTimes: Map<string, number>, expectedSeq: number): Promise<void> {
    const extractor = getOCRExtractor();
    if (!extractor || !document.body) {
        return;
    }

    const images = collectImages(document.body);
    if (images.length === 0) {
        return;
    }

    let anyImageFlagged = false;
    for (const img of images) {
        if (scanSequence !== expectedSeq) {
            return;
        }

        const imageKey = getImageScanKey(img);
        const imageSource = img.currentSrc || img.src;
        if (img.classList.contains(IMAGE_CLASS) && img.dataset.judolImageKey !== imageKey) {
            restoreImage(img);
        }

        try {
            const metadataFlagged = processOCRResult(
                { imageElement: img, extractedText: getImageMetadataText(img) },
                KEYWORDS,
                stats,
                keywordAlgorithmTimes,
            );
            if (metadataFlagged) {
                markImageProcessed(img, imageKey);
                anyImageFlagged = true;
                continue;
            }

            const extractedText = await getCachedOcrText(img, extractor);
            if (scanSequence !== expectedSeq || (img.currentSrc || img.src) !== imageSource) {
                return;
            }
            markImageProcessed(img, getImageScanKey(img));
            const flagged = processOCRResult({imageElement: img, extractedText}, KEYWORDS, stats, keywordAlgorithmTimes);
            if (flagged) {
                anyImageFlagged = true;
            }
        } catch {
            // OCR failure is non-fatal
        }
    }
    if (anyImageFlagged && scanSequence === expectedSeq) {
        publishScanStatistics(stats);
    }
}

function getImageMetadataText(img: HTMLImageElement): string {
    const source = img.currentSrc || img.src;
    const parts = [
        img.alt,
        img.title,
        img.getAttribute("aria-label") ?? "",
        getReadableUrlText(source),
        isStandaloneImageDocument(img) ? document.title : "",
    ];

    return parts.filter(Boolean).join(" ");
}

function getReadableUrlText(value: string): string {
    if (!value) {
        return "";
    }

    try {
        const url = new URL(value, window.location.href);
        const filename = url.pathname.split("/").filter(Boolean).at(-1) ?? "";
        return decodeURIComponent(filename)
            .replace(/\.[a-z0-9]+$/iu, "")
            .replace(/[-_+.]+/gu, " ");
    } catch {
        return value.replace(/[-_+.]+/gu, " ");
    }
}

function isStandaloneImageDocument(img: HTMLImageElement): boolean {
    return document.body.childElementCount === 1 && document.body.firstElementChild === img;
}

function getCachedOcrText(img: HTMLImageElement, extractor: NonNullable<ReturnType<typeof getOCRExtractor>>): Promise<string> {
    const cacheKey = img.currentSrc || img.src;
    const existing = ocrTextCache.get(cacheKey);
    if (existing) {
        return existing;
    }

    const job = extractor(img).catch((error: unknown) => {
        ocrTextCache.delete(cacheKey);
        throw error;
    });
    ocrTextCache.set(cacheKey, job);
    trimOcrCache();
    return job;
}

function trimOcrCache(): void {
    while (ocrTextCache.size > maxOcrCacheEntries) {
        const oldestKey = ocrTextCache.keys().next().value;
        if (oldestKey === undefined) {
            return;
        }
        ocrTextCache.delete(oldestKey);
    }
}

function scheduleImageScan(job: ImageScanJob): void {
    pendingImageScan = job;
    void drainImageScanQueue();
}

async function drainImageScanQueue(): Promise<void> {
    if (imageScanRunning) {
        return;
    }

    imageScanRunning = true;
    try {
        while (pendingImageScan !== null) {
            const job = pendingImageScan;
            pendingImageScan = null;
            await runImageScan(job.stats, job.keywordAlgorithmTimes, job.expectedSeq);
        }
    } finally {
        imageScanRunning = false;
    }
}
