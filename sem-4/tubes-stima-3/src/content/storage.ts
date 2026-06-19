import { SETTINGS_KEY, STORAGE_KEY } from "./constants.ts";
import type { ChromeLike, ExtensionMessage, ExtensionSettings, ScanStatistics } from "./types.ts";

export const DEFAULT_EXTENSION_SETTINGS: ExtensionSettings = {
    blurTextEnabled: true,
};

export function publishScanStatistics(stats: ScanStatistics): void {
    try {
        const api = getExtensionChrome();
        api?.storage?.local?.set({ [STORAGE_KEY]: stats });
        if (stats.pageUrl) {
            const urlKey = `${STORAGE_KEY}::${stats.pageUrl}`;
            api?.storage?.local?.set({ [urlKey]: stats });
        }
    } catch {
        // Extension context invalidated (extension was reloaded while page is still open)
    }
}

export function addRescanListener(onRescan: () => void): void {
    const api = getExtensionChrome();

    api?.runtime?.onMessage?.addListener((message: ExtensionMessage, _sender, sendResponse) => {
        if (message.type !== "JUDOL_RESCAN_REQUEST") {
            return false;
        }

        onRescan();
        sendResponse({ ok: true });
        return true;
    });
}

export function loadExtensionSettings(onLoaded: (settings: ExtensionSettings) => void): void {
    try {
        const api = getExtensionChrome();

        if (!api?.storage?.local?.get) {
            onLoaded(DEFAULT_EXTENSION_SETTINGS);
            return;
        }

        api.storage.local.get({ [SETTINGS_KEY]: DEFAULT_EXTENSION_SETTINGS }, (items) => {
            onLoaded(normalizeExtensionSettings(items[SETTINGS_KEY]));
        });
    } catch {
        onLoaded(DEFAULT_EXTENSION_SETTINGS);
    }
}

export function saveExtensionSettings(settings: ExtensionSettings, onSaved?: () => void): void {
    try {
        const api = getExtensionChrome();
        if (!api?.storage?.local?.set) {
            onSaved?.();
            return;
        }

        api.storage.local.set({ [SETTINGS_KEY]: normalizeExtensionSettings(settings) }, onSaved);
    } catch {
        onSaved?.();
    }
}

export function addSettingsListener(onSettingsChanged: (settings: ExtensionSettings) => void): void {
    const api = getExtensionChrome();

    api?.storage?.onChanged?.addListener((changes, areaName) => {
        if (areaName !== "local") {
            return;
        }

        const change = changes[SETTINGS_KEY];
        if (!change) {
            return;
        }

        onSettingsChanged(normalizeExtensionSettings(change.newValue));
    });
}

export function normalizeExtensionSettings(value: unknown): ExtensionSettings {
    const settings = value as Partial<ExtensionSettings> | undefined;

    return {
        blurTextEnabled: typeof settings?.blurTextEnabled === "boolean"
            ? settings.blurTextEnabled
            : DEFAULT_EXTENSION_SETTINGS.blurTextEnabled,
    };
}

function getExtensionChrome(): ChromeLike | undefined {
    return (globalThis as unknown as { chrome?: ChromeLike }).chrome;
}
