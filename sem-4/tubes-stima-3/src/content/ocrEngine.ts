import { createWorker, type Worker } from "tesseract.js";

let worker: Worker | null = null;
let workerReady: Promise<Worker> | null = null;
const IMAGE_LOAD_TIMEOUT_MS = 5000;
const MAX_OCR_EDGE_PX = 1200;
const MAX_OCR_PIXELS = 1_000_000;

// initialises a single shared Tesseract worker
function getWorker(): Promise<Worker> {
    if (workerReady) {
        return workerReady;
    }

    workerReady = createWorker("eng+ind").then((w) => {
        worker = w;
        return w;
    });

    return workerReady;
}

// extracts text from an image element w tesseract.js OCR
export async function extractText(img: HTMLImageElement): Promise<string> {
    if (!img.complete) {
        await waitForLoad(img);
    }
    if (img.naturalWidth === 0 || img.naturalHeight === 0) {
        return "";
    }

    // draw to canvas so we own the pixel data (avoids CORS blob issues)
    const canvas = imageToCanvas(img);
    if (!canvas) {
        return "";
    }

    const w = await getWorker();
    const { data } = await w.recognize(canvas);
    return data.text;
}

// terminates shared worker
export async function terminateOCR(): Promise<void> {
    if (worker) {
        await worker.terminate();
        worker = null;
        workerReady = null;
    }
}


function waitForLoad(img: HTMLImageElement): Promise<void> {
    return new Promise<void>((resolve) => {
        if (img.complete) {
            resolve();
            return;
        }

        let timeoutId: number | undefined;
        const cleanup = () => {
            img.removeEventListener("load", onDone);
            img.removeEventListener("error", onDone);
            if (timeoutId !== undefined) {
                window.clearTimeout(timeoutId);
            }
        };
        const onDone = () => { cleanup(); resolve(); };

        img.addEventListener("load", onDone, { once: true });
        img.addEventListener("error", onDone, { once: true });
        timeoutId = window.setTimeout(onDone, IMAGE_LOAD_TIMEOUT_MS);
    });
}

// returns a canvas with the image drawn, or null on cross-origin / security errors
function imageToCanvas(img: HTMLImageElement): HTMLCanvasElement | null {
    try {
        const dimensions = getOcrCanvasDimensions(img.naturalWidth, img.naturalHeight);
        const canvas = document.createElement("canvas");
        canvas.width = dimensions.width;
        canvas.height = dimensions.height;

        const ctx = canvas.getContext("2d");
        if (!ctx) {
            return null;
        }

        ctx.drawImage(img, 0, 0, dimensions.width, dimensions.height);

        // probe for tainted canvas (cross-origin without CORS headers)
        ctx.getImageData(0, 0, 1, 1);

        return canvas;
    } catch {
        // SecurityError from tainted canvas (skip this image)
        return null;
    }
}

function getOcrCanvasDimensions(width: number, height: number): { width: number; height: number } {
    const edgeScale = Math.min(1, MAX_OCR_EDGE_PX / Math.max(width, height));
    const pixelScale = Math.min(1, Math.sqrt(MAX_OCR_PIXELS / Math.max(1, width * height)));
    const scale = Math.min(edgeScale, pixelScale);

    return {
        width: Math.max(1, Math.round(width * scale)),
        height: Math.max(1, Math.round(height * scale)),
    };
}
