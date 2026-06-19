import type { ChartDatum, ScanStatistics } from "../content/types.ts";

const palette = ["#1f7a5c", "#d9480f", "#2f6fbb", "#b08900", "#7c5c3b", "#4d7c0f", "#9f1239", "#475569"];

export function renderKeywordFrequencyChart(canvas: HTMLCanvasElement, stats: ScanStatistics | null): void {
    const data = stats?.keywordFrequency.map((item, index) => ({
        label: item.keyword,
        value: item.occurrences,
        color: palette[index % palette.length],
    })) ?? [];

    const minWidth = 350;
    const barWidthEstimate = 46; // width per bar + gap
    const calculatedWidth = Math.max(minWidth, data.length * barWidthEstimate + 50);
    canvas.style.width = `${calculatedWidth}px`;

    renderBarChart(canvas, data, "Keyword Frequency", "occ");
}

function renderBarChart(canvas: HTMLCanvasElement, data: ChartDatum[], title: string, unit: string): void {
    const context = canvas.getContext("2d");

    if (!context) {
        return;
    }

    const pixelRatio = window.devicePixelRatio || 1;
    const width = canvas.clientWidth || 350;
    const height = canvas.clientHeight || 170;
    canvas.width = Math.round(width * pixelRatio);
    canvas.height = Math.round(height * pixelRatio);
    context.setTransform(pixelRatio, 0, 0, pixelRatio, 0, 0);
    context.clearRect(0, 0, width, height);
    drawTitle(context, title);

    const visibleData = data.filter((item) => item.value > 0);

    if (visibleData.length === 0) {
        drawEmptyState(context, width, height);
        return;
    }

    const chartTop = 38;
    const chartBottom = height - 34;
    const chartLeft = 36;
    const chartRight = width - 14;
    const chartHeight = chartBottom - chartTop;
    const maxValue = Math.max(...visibleData.map((item) => item.value));
    const gap = 7;
    const barWidth = Math.max(12, (chartRight - chartLeft - gap * (visibleData.length - 1)) / visibleData.length);

    drawAxis(context, chartLeft, chartTop, chartBottom, chartRight);

    visibleData.forEach((item, index) => {
        const x = chartLeft + index * (barWidth + gap);
        const barHeight = maxValue === 0 ? 0 : Math.max(2, (item.value / maxValue) * chartHeight);
        const y = chartBottom - barHeight;

        context.fillStyle = item.color ?? palette[index % palette.length];
        context.fillRect(x, y, barWidth, barHeight);
        context.fillStyle = "#1c1f1a";
        context.font = "11px system-ui, sans-serif";
        context.textAlign = "center";
        context.fillText(formatNumber(item.value), x + barWidth / 2, Math.max(30, y - 5));
        context.save();
        context.translate(x + barWidth / 2, height - 11);
        context.rotate(-0.42);
        context.fillStyle = "#5d6759";
        context.fillText(shortenLabel(item.label), 0, 0);
        context.restore();
    });

    context.fillStyle = "#6a7266";
    context.font = "10px system-ui, sans-serif";
    context.textAlign = "left";
    context.fillText(unit, chartLeft, chartTop - 6);
}

function drawTitle(context: CanvasRenderingContext2D, title: string): void {
    context.fillStyle = "#171a15";
    context.font = "700 13px system-ui, sans-serif";
    context.textAlign = "left";
    context.fillText(title, 12, 21);
}

function drawAxis(context: CanvasRenderingContext2D, left: number, top: number, bottom: number, right: number): void {
    context.strokeStyle = "#d8ded1";
    context.lineWidth = 1;
    context.beginPath();
    context.moveTo(left, top);
    context.lineTo(left, bottom);
    context.lineTo(right, bottom);
    context.stroke();
}

function drawEmptyState(context: CanvasRenderingContext2D, width: number, height: number): void {
    context.fillStyle = "#727a6e";
    context.font = "12px system-ui, sans-serif";
    context.textAlign = "center";
    context.fillText("No data", width / 2, height / 2 + 6);
}

function formatNumber(value: number): string {
    if (value >= 1000) {
        return `${(value / 1000).toFixed(1)}k`;
    }

    return String(value);
}

function shortenLabel(label: string): string {
    return label.length > 12 ? `${label.slice(0, 10)}..` : label;
}
