import { defineConfig } from "vite";
import preact from "@preact/preset-vite";
import { ViteEjsPlugin } from "vite-plugin-ejs";
import tailwindcss from "@tailwindcss/vite";

// https://vitejs.dev/config/
export default defineConfig({
  plugins: [preact(), ViteEjsPlugin(), tailwindcss()],
  build: {
    rollupOptions: {
      output: {
        entryFileNames: "[name].js",
        chunkFileNames: "[name].js",
        assetFileNames: "[name].[ext]",
      },
    },
  },
});
