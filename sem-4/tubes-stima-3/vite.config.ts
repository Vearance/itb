import { defineConfig } from "vite";
import { resolve } from "path";
import { fileURLToPath } from "url";

const __dirname = fileURLToPath(new URL(".", import.meta.url));

export default defineConfig({
    build: {
        outDir: "dist",
        rollupOptions: {
            input: {
                content: resolve(__dirname, "src/content/content.ts"),
                popup: resolve(__dirname, "src/popup/popup.html"),
            },
            output: {
                entryFileNames: "[name].js",
            },
        },
    },
});
