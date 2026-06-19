import { IMAGE_CLASS } from "./constants.ts";

const MIN_IMAGE_DIMENSION_PX = 50; // min dimension to skip tracking pixels and tiny decorative icons
const SKIP_IMG_PARENTS = new Set(["NOSCRIPT", "TEMPLATE"]); // parent tags whose children should not be scanned

// returns images that have not been processed for their current source/dimensions.
export function collectImages(root: HTMLElement): HTMLImageElement[] {
    const images: HTMLImageElement[] = [];
    for (const img of Array.from(root.querySelectorAll<HTMLImageElement>("img"))) {
        if (!isImageScannable(img)) {
            continue;
        }
        images.push(img);
    }
    return images;
}

export function getImageScanKey(img: HTMLImageElement): string {
    return `${img.currentSrc || img.src}|${img.naturalWidth}x${img.naturalHeight}`;
}

// marks an image so it won't be re-queued while its source/dimensions stay unchanged
export function markImageProcessed(img: HTMLImageElement, imageKey = getImageScanKey(img)): void {
    img.dataset.judolDetected = "true";
    img.dataset.judolImageKey = imageKey;
}

export function isImageProcessed(img: HTMLImageElement): boolean {
    return img.classList.contains(IMAGE_CLASS) && img.dataset.judolDetected === "true" && img.dataset.judolImageKey === getImageScanKey(img);
}

// removes the processed guard flag so the image can be re-evaluated
export function clearImageProcessedFlag(img: HTMLImageElement): void {
    delete img.dataset.judolDetected;
    delete img.dataset.judolImageKey;
}

function isImageScannable(img: HTMLImageElement): boolean {
    if (isImageProcessed(img)) {
        return false;
    }
    if (img.classList.contains(IMAGE_CLASS) && img.dataset.judolImageKey === getImageScanKey(img)) {
        return false;
    }

    const parent = img.parentElement;
    if (parent !== null && SKIP_IMG_PARENTS.has(parent.tagName)) {
        return false;
    }

    if (!img.src && !img.currentSrc) {
        return false;
    }

    // skip tiny loaded images, unloaded ones are still candidates
    if (img.naturalWidth > 0 && img.naturalWidth < MIN_IMAGE_DIMENSION_PX) {
        return false;
    }
    if (img.naturalHeight > 0 && img.naturalHeight < MIN_IMAGE_DIMENSION_PX) {
        return false;
    }
    return true;
}