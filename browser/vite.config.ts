import { readFileSync } from "node:fs";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";
import { defineConfig, loadEnv } from "vite";
import preact from "@preact/preset-vite";
import { ViteEjsPlugin } from "vite-plugin-ejs";
import tailwindcss from "@tailwindcss/vite";

const projectDir = dirname(fileURLToPath(import.meta.url));

/**
 * Fall back to the secret the release tooling uses, so `npm run dev` works
 * without extra setup. Only read here, in the dev server process — the app
 * itself never receives this value.
 */
function releaseEnv() {
  try {
    const env = readFileSync(resolve(projectDir, "../release/.env"), "utf8");
    return env.match(/^\s*BEARER_TOKEN\s*=\s*(.+?)\s*$/m)?.[1] ?? "";
  } catch {
    return "";
  }
}

// https://vitejs.dev/config/
export default defineConfig(({ mode }) => {
  const env = loadEnv(mode, process.cwd(), "");
  // Precedence: browser/.env.local, then the shell, then release/.env.
  const releaseToken =
    env.VITE_RELEASE_TOKEN || process.env.VITE_RELEASE_TOKEN || releaseEnv();

  return {
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
    server: {
      proxy: {
        // Keeps the dev server same-origin with the release channel, so no CORS
        // preflight is involved and the token stays out of the browser. The
        // server accepts this bearer token or Basic with the same secret.
        "/pixelmatrix": {
          target: "https://hannes-linder.com",
          changeOrigin: true,
          secure: true,
          configure: (proxy) => {
            proxy.on("proxyReq", (proxyReq) => {
              if (releaseToken) {
                proxyReq.setHeader("Authorization", `Bearer ${releaseToken}`);
              }
            });
          },
        },
      },
    },
  };
});
