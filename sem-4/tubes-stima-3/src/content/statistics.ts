import { ALGORITHMS, type AlgorithmTypes, type MatchResult, type RecordAlgorithmOptions, type ScanStatistics, type TextHighlightPlan } from "./types";

// Membuat objek data statistik awal (kosong) sebelum pemindaian halaman web dilakukan
export function createEmptyScanStatistics(scanId: string): ScanStatistics {
    const algorithms = {} as ScanStatistics["algorithms"];

    for (const algorithm of ALGORITHMS) {
        algorithms[algorithm] = {algorithm, matches: 0, executionTime: 0, comparisonCount: 0};
    }

    return {
        scanId, scannedAt: Date.now(), pageUrl: window.location.href, pageTitle: document.title || window.location.hostname || "Untitled page",
        totalKeywordsFound: 0, totalMatches: 0, algorithms, keywordFrequency: []};
}

// Mencatat waktu eksekusi, jumlah kecocokan, dan perbandingan karakter dari hasil pencarian setiap kata kunci
export function recordAlgorithmResult(
    stats: ScanStatistics,
    result: MatchResult,
    keyword: string,
    keywordAlgorithmTimes: Map<string, number>,
    options: RecordAlgorithmOptions = {},
): void {
    const algorithmStats = stats.algorithms[result.algorithm];
    algorithmStats.matches += result.matchIndexes.length;

    if (options.countCost ?? true) {
        algorithmStats.executionTime += result.executionTime;
        algorithmStats.comparisonCount += result.comparisonCount;
    }

    const timeKey = getKeywordAlgorithmKey(keyword, result.algorithm);
    keywordAlgorithmTimes.set(timeKey, (keywordAlgorithmTimes.get(timeKey) ?? 0) + result.executionTime);
}

export function recordAlgorithmExecution(
    stats: ScanStatistics,
    algorithm: AlgorithmTypes,
    executionTime: number,
    comparisonCount: number,
): void {
    const algorithmStats = stats.algorithms[algorithm];
    algorithmStats.executionTime += executionTime;
    algorithmStats.comparisonCount += comparisonCount;
}

export function recordAlgorithmMatches(stats: ScanStatistics, algorithm: AlgorithmTypes, matches: number): void {
    stats.algorithms[algorithm].matches += matches;
}

// Menyelesaikan kompilasi data statistik akhir dan menghitung daftar frekuensi kata kunci yang ditemukan
export function finalizeScanStatistics(stats: ScanStatistics, plans: TextHighlightPlan[]): void {
    const frequency = new Map<string, number>();

    for (const plan of plans) {
        frequency.set(plan.keyword, (frequency.get(plan.keyword) ?? 0) + 1);
    }

    stats.totalMatches = plans.length;
    stats.totalKeywordsFound = frequency.size;
    stats.keywordFrequency = Array.from(frequency.entries()).map(([keyword, occurrences]) => ({ keyword, occurrences })).sort((a, b) => b.occurrences - a.occurrences || a.keyword.localeCompare(b.keyword));
}

// Menempelkan metrik akhir (jumlah kemunculan dan waktu eksekusi) pada rencana highlight setiap node teks
export function applyFinalHighlightMetadata(plans: TextHighlightPlan[], keywordAlgorithmTimes: Map<string, number>): void {
    const frequency = new Map<string, number>();

    for (const plan of plans) {
        frequency.set(plan.keyword, (frequency.get(plan.keyword) ?? 0) + 1);
    }

    for (const plan of plans) {
        plan.occurrences = frequency.get(plan.keyword) ?? 1;
        plan.executionTime = keywordAlgorithmTimes.get(getKeywordAlgorithmKey(plan.keyword, plan.algorithm)) ?? plan.executionTime;
    }
}

// Memperbarui statistik total untuk gambar yang terdeteksi oleh OCR (dipanggil secara async)
export function recordImageMatch(stats: ScanStatistics, keyword: string, occurrences: number): void {
    stats.totalMatches += occurrences;

    const existing = stats.keywordFrequency.find((entry) => entry.keyword === keyword);

    if (existing) {
        existing.occurrences += occurrences;
    } else {
        stats.totalKeywordsFound++;
        stats.keywordFrequency.push({ keyword, occurrences });
    }

    // Keep frequency list sorted descending by occurrences
    stats.keywordFrequency.sort(
        (a, b) => b.occurrences - a.occurrences || a.keyword.localeCompare(b.keyword),
    );
}

// Membuat kunci unik gabungan untuk memetakan performa per kata kunci per jenis algoritma
function getKeywordAlgorithmKey(keyword: string, algorithm: AlgorithmTypes): string {
    return `${algorithm}::${keyword}`;
}
