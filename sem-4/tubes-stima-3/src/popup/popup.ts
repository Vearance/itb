import { renderKeywordFrequencyChart } from "./visualization.ts";
import type { ChromeLike, ExtensionSettings, ScanStatistics } from "../content/types.ts";

const STORAGE_KEY = "judol-scan-stats";
const SETTINGS_KEY = "judol-detector-settings";
const DEFAULT_EXTENSION_SETTINGS: ExtensionSettings = {
    blurTextEnabled: true,
};

const pageTitleElement = getRequiredElement<HTMLParagraphElement>("page-title");
const totalKeywordsElement = getRequiredElement<HTMLElement>("total-keywords");
const totalMatchesElement = getRequiredElement<HTMLElement>("total-matches");
const lastScanElement = getRequiredElement<HTMLElement>("last-scan");
const algorithmStatsElement = getRequiredElement<HTMLElement>("algorithm-stats");
const statusElement = getRequiredElement<HTMLParagraphElement>("status");
const rescanButton = getRequiredElement<HTMLButtonElement>("rescan-button");
const blurToggle = getRequiredElement<HTMLInputElement>("blur-toggle");
const keywordChart = getRequiredElement<HTMLCanvasElement>("keyword-chart");

let activeTabUrl: string | undefined = undefined;

initializePopup();

function initializePopup(): void {
    renderEmptyState();
    loadStoredSettings();
    
    const api = getExtensionChrome();
    if (api?.tabs?.query) {
        api.tabs.query({ active: true, currentWindow: true }, (tabs) => {
            const tab = tabs[0];
            activeTabUrl = tab?.url;
            
            loadStoredStats();
            subscribeToRealtimeStats();
        });
    } else {
        loadStoredStats();
        subscribeToRealtimeStats();
    }

    rescanButton.addEventListener("click", requestRescan);
    blurToggle.addEventListener("change", handleBlurToggleChange);
}

function loadStoredStats(): void {
    const api = getExtensionChrome();

    if (!api?.storage?.local?.get) {
        requestRescan();
        return;
    }

    const urlKey = activeTabUrl ? `${STORAGE_KEY}::${activeTabUrl}` : undefined;
    const keys = urlKey ? [STORAGE_KEY, urlKey] : [STORAGE_KEY];

    api.storage.local.get(keys, (items) => {
        let stats = urlKey ? items[urlKey] as ScanStatistics | undefined : undefined;
        
        // Fallback jika tidak ada data spesifik URL, tapi ada data global yang cocok
        if (!stats) {
            const globalStats = items[STORAGE_KEY] as ScanStatistics | undefined;
            if (globalStats && (!activeTabUrl || globalStats.pageUrl === activeTabUrl)) {
                stats = globalStats;
            }
        }

        if (stats) {
            renderStats(stats);
        } else {
            requestRescan();
        }
    });
}

function subscribeToRealtimeStats(): void {
    const api = getExtensionChrome();

    api?.storage?.onChanged?.addListener((changes, areaName) => {
        if (areaName !== "local") {
            return;
        }

        const settingsChange = changes[SETTINGS_KEY];
        if (settingsChange) {
            renderSettings(normalizeExtensionSettings(settingsChange.newValue));
        }

        const urlKey = activeTabUrl ? `${STORAGE_KEY}::${activeTabUrl}` : undefined;
        const pageStats = urlKey ? changes[urlKey]?.newValue : undefined;
        const updatedStats = (pageStats ?? changes[STORAGE_KEY]?.newValue) as ScanStatistics | undefined;

        if (updatedStats && (!activeTabUrl || updatedStats.pageUrl === activeTabUrl)) {
            renderStats(updatedStats);
        }
    });
}

function loadStoredSettings(): void {
    const api = getExtensionChrome();

    if (!api?.storage?.local?.get) {
        renderSettings(DEFAULT_EXTENSION_SETTINGS);
        return;
    }

    api.storage.local.get({ [SETTINGS_KEY]: DEFAULT_EXTENSION_SETTINGS }, (items) => {
        renderSettings(normalizeExtensionSettings(items[SETTINGS_KEY]));
    });
}

function renderSettings(settings: ExtensionSettings): void {
    blurToggle.checked = settings.blurTextEnabled;
}

function handleBlurToggleChange(): void {
    const settings: ExtensionSettings = {
        ...DEFAULT_EXTENSION_SETTINGS,
        blurTextEnabled: blurToggle.checked,
    };

    saveExtensionSettings(settings, () => {
        statusElement.textContent = settings.blurTextEnabled ? "Text blur enabled" : "Text blur disabled";
        requestRescan();
    });
}

function saveExtensionSettings(settings: ExtensionSettings, onSaved?: () => void): void {
    const api = getExtensionChrome();

    if (!api?.storage?.local?.set) {
        onSaved?.();
        return;
    }

    api.storage.local.set({ [SETTINGS_KEY]: normalizeExtensionSettings(settings) }, onSaved);
}

function normalizeExtensionSettings(value: unknown): ExtensionSettings {
    const settings = value as Partial<ExtensionSettings> | undefined;

    return {
        blurTextEnabled: typeof settings?.blurTextEnabled === "boolean"
            ? settings.blurTextEnabled
            : DEFAULT_EXTENSION_SETTINGS.blurTextEnabled,
    };
}

function renderStats(stats: ScanStatistics): void {
    pageTitleElement.textContent = stats.pageTitle;
    pageTitleElement.title = stats.pageUrl;
    totalKeywordsElement.textContent = String(stats.totalKeywordsFound);
    totalMatchesElement.textContent = String(stats.totalMatches);
    lastScanElement.textContent = formatTime(stats.scannedAt);
    statusElement.textContent = `Last scan: ${formatDate(stats.scannedAt)}`;

    renderAlgorithmStats(stats);
    renderKeywordFrequencyChart(keywordChart, stats);
}

function renderAlgorithmStats(stats: ScanStatistics): void {
    const rows = Object.values(stats.algorithms).map((algorithm) => {
        const row = document.createElement("div");
        const label = document.createElement("strong");
        const value = document.createElement("span");

        row.className = "stat-row";
        label.textContent = algorithm.algorithm;
        value.textContent = `${algorithm.matches} matches | ${algorithm.executionTime.toFixed(3)} ms`;
        row.append(label, value);
        return row;
    });

    algorithmStatsElement.replaceChildren(...rows);
}

function renderEmptyState(): void {
    const empty = document.createElement("p");
    empty.className = "empty";
    empty.textContent = "No scan data yet";

    algorithmStatsElement.replaceChildren(empty);
    renderKeywordFrequencyChart(keywordChart, null);
}

function requestRescan(): void {
    const api = getExtensionChrome();

    if (!api?.tabs?.query || !api.tabs.sendMessage) {
        statusElement.textContent = "Rescan unavailable in this browser context";
        return;
    }

    rescanButton.disabled = true;
    statusElement.textContent = "Requesting rescan...";

    api.tabs.query({ active: true, currentWindow: true }, (tabs) => {
        const tabId = tabs[0]?.id;

        if (typeof tabId !== "number" || !api.tabs?.sendMessage) {
            rescanButton.disabled = false;
            statusElement.textContent = "No active tab found";
            return;
        }

        api.tabs.sendMessage(tabId, { type: "JUDOL_RESCAN_REQUEST" }, (response) => {
            rescanButton.disabled = false;
            if (api.runtime?.lastError?.message) {
                statusElement.textContent = api.runtime.lastError.message;
                return;
            }

            statusElement.textContent = response ? "Scan queued" : "Rescan requested";
        });
    });
}

function getRequiredElement<T extends HTMLElement>(id: string): T {
    const element = document.getElementById(id);

    if (!(element instanceof HTMLElement)) {
        throw new Error(`Missing popup element: ${id}`);
    }

    return element as T;
}

function getExtensionChrome(): ChromeLike | undefined {
    return (globalThis as unknown as { chrome?: ChromeLike }).chrome;
}

function formatTime(timestamp: number): string {
    return new Date(timestamp).toLocaleTimeString([], {
        hour: "2-digit",
        minute: "2-digit",
    });
}

function formatDate(timestamp: number): string {
    return new Date(timestamp).toLocaleString([], {
        dateStyle: "medium",
        timeStyle: "medium",
    });
}
