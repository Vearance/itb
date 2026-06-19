import { IMAGE_CLASS } from "./constants.ts";
import { regexMatcher } from "../algorithms/regex.ts";
import { clearImageProcessedFlag, markImageProcessed } from "./imageDetection.ts";
import { matchKeyword, runAhoCorasickOnce } from "./matchers.ts";
import { recordAlgorithmExecution, recordAlgorithmMatches, recordAlgorithmResult, recordImageMatch } from "./statistics.ts";
import { toCanonicalText } from "./unicode.ts";
import type { AlgorithmTypes, HighlightMetadata, ScanStatistics, TextMatch } from "./types.ts";

// result delivered by the OCR module
export interface OCRResult {
    imageElement: HTMLImageElement;
    extractedText: string;
}

//returns the raw text extracted from the image or an empty string if extraction fails
export type OCRExtractor = (img: HTMLImageElement) => Promise<string>;
let _ocrExtractor: OCRExtractor | null = null;

//register OCR extractor once the ocr module is loaded
export function registerOCRExtractor(fn: OCRExtractor): void {
    _ocrExtractor = fn;
}

//get OCR extractor
export function getOCRExtractor(): OCRExtractor | null {
    return _ocrExtractor;
}

// runs keyword matching on the OCR-extracted text
export function processOCRResult(result: OCRResult, keywords: string[], stats: ScanStatistics, keywordAlgorithmTimes: Map<string, number>): boolean {
    const { imageElement: img, extractedText } = result;

    if (!extractedText.trim()) {
        return false;
    }

    let bestKeyword: string | null = null;
    let bestAlgorithm: AlgorithmTypes = "KMP";
    let bestExecTime = 0;
    let totalImageMatches = 0;
    const countedRanges: MatchRange[] = [];

    // pre-compute canonical text dan A-C results sekali untuk OCR text
    const canonical = toCanonicalText(extractedText);
    const acResult = runAhoCorasickOnce(canonical.value);
    recordAlgorithmExecution(stats, "AhoCorasick", acResult.elapsedMs, acResult.comparisonCount);

    const regexOutput = regexMatcher(extractedText, canonical);
    recordAlgorithmExecution(stats, "RegEx", regexOutput.executionMs, extractedText.length);
    recordAlgorithmMatches(stats, "RegEx", regexOutput.matches.length);
    totalImageMatches += countNewTextMatches(countedRanges, regexOutput.matches);
    if (regexOutput.matches.length > 0) {
        bestKeyword = regexOutput.matches[0].keyword;
        bestAlgorithm = "RegEx";
        bestExecTime = regexOutput.executionMs;
    }

    for (const keyword of keywords) {
        const results = matchKeyword(extractedText, keyword, canonical, acResult.matches, acResult.comparisonCount, acResult.elapsedMs);

        for (const matchResult of results) {
            recordAlgorithmResult(stats, matchResult, keyword, keywordAlgorithmTimes, {
                countCost: !isSharedTextAlgorithm(matchResult.algorithm),
            });

            if (matchResult.matched && matchResult.matchIndexes.length > 0) {
                totalImageMatches += countNewMatchRanges(countedRanges, matchResult, keyword);

                if (bestKeyword === null) {
                    bestKeyword = keyword;
                    bestAlgorithm = matchResult.algorithm;
                    bestExecTime = matchResult.executionTime;
                }
            }
        }
    }

    if (bestKeyword === null) {
        return false;
    }

    const metadata: HighlightMetadata = {
        keyword: bestKeyword, algorithm: bestAlgorithm,
        occurrences: totalImageMatches, executionTime: bestExecTime};

    blurImage(img, metadata);
    recordImageMatch(stats, bestKeyword, totalImageMatches);
    return true;
}

function isSharedTextAlgorithm(algorithm: AlgorithmTypes): boolean {
    return algorithm === "AhoCorasick";
}

interface MatchRange {
    start: number;
    end: number;
}

function countNewTextMatches(countedRanges: MatchRange[], matches: TextMatch[]): number {
    let newRanges = 0;

    for (const match of matches) {
        if (tryCountRange(countedRanges, match.start, match.end)) {
            newRanges++;
        }
    }

    return newRanges;
}

function countNewMatchRanges(countedRanges: MatchRange[], result: { matchIndexes: number[]; matchLengths?: number[] }, keyword: string): number {
    let newRanges = 0;

    for (let i = 0; i < result.matchIndexes.length; i++) {
        const start = result.matchIndexes[i];
        const length = result.matchLengths?.[i] ?? keyword.length;

        if (tryCountRange(countedRanges, start, start + length)) {
            newRanges++;
        }
    }

    return newRanges;
}

function tryCountRange(countedRanges: MatchRange[], start: number, end: number): boolean {
    if (countedRanges.some((range) => start < range.end && range.start < end)) {
        return false;
    }

    countedRanges.push({ start, end });
    return true;
}

// blur image with tooltip metadata
export function blurImage(img: HTMLImageElement, metadata: HighlightMetadata): void {
    if (img.classList.contains(IMAGE_CLASS)) {
        return;
    }

    if (img.dataset.originalFilter === undefined) {
        img.dataset.originalFilter = img.style.filter;
    }
    
    img.dataset.judolAction = "blur";
    attachImageMetadata(img, metadata);
    img.classList.add(IMAGE_CLASS);
    markImageProcessed(img);
}

// reverses the blur applied by blurImage
export function restoreImage(img: HTMLImageElement): void {
    if (img.dataset.judolAction === "blur") {
        img.style.filter = img.dataset.originalFilter ?? "";
        delete img.dataset.originalFilter;
    }

    img.classList.remove(IMAGE_CLASS);
    delete img.dataset.keyword;
    delete img.dataset.algorithm;
    delete img.dataset.occurrences;
    delete img.dataset.executionTime;
    delete img.dataset.judolAction;
    clearImageProcessedFlag(img);
}

// called at the start of every rescan to strip image highlights from the previous cycle
export function removeOldImageHighlights(root: ParentNode = document.body): void {
    const flaggedImages = Array.from(
        root.querySelectorAll<HTMLImageElement>(`img.${IMAGE_CLASS}`),
    );

    for (const img of flaggedImages) {
        restoreImage(img);
    }
}

function attachImageMetadata(img: HTMLImageElement, metadata: HighlightMetadata): void {
    img.dataset.keyword = metadata.keyword;
    img.dataset.algorithm = metadata.algorithm;
    img.dataset.occurrences = String(metadata.occurrences);
    img.dataset.executionTime = metadata.executionTime.toFixed(3);
}
