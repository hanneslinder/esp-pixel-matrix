import { useCallback, useEffect, useState } from "react";
import { config } from "../../../config";

const localVersion = (window as any).version;

const hasBearerToken = () => Boolean(config.updates.bearerToken);

const authHeaders = (): HeadersInit | undefined =>
  hasBearerToken()
    ? { Authorization: `Bearer ${config.updates.bearerToken}` }
    : undefined;

export const VersionChecker = () => {
  const [didUpdateCheck, setUpdateCheck] = useState(false);
  const [versionNumber, setNewVersionNumber] = useState(localVersion);
  const [newVersionAvailable, setNewVersionAvailable] = useState(false);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    if (!config.updates.versionUrl) {
      setUpdateCheck(true);
      return;
    }

    const controller = new AbortController();

    fetch(config.updates.versionUrl, {
      method: "GET",
      headers: authHeaders(),
      cache: "no-store",
      signal: controller.signal,
    })
      .then((response) => {
        if (!response.ok) {
          throw new Error(`update check failed with ${response.status}`);
        }
        return response.json();
      })
      .then((json) => {
        const latest = Array.isArray(json) ? json[0] : json;
        setUpdateCheck(true);

        if (latest?.version && latest.version !== localVersion) {
          setNewVersionNumber(latest.version);
          setNewVersionAvailable(true);
        }
      })
      .catch((err) => {
        if (err.name === "AbortError") return;
        // A failed update check must never break the app itself.
        console.warn("Update check failed:", err.message);
        setError(err.message);
        setUpdateCheck(true);
      });

    return () => controller.abort();
  }, []);

  /**
   * Open the download page as a normal page, not as a downloaded file. A
   * navigation cannot carry an Authorization: Bearer header, so the URL carries
   * Basic credentials instead — the server accepts either (same secret), and a
   * browser will use credentials embedded in the URL without prompting.
   */
  const openDownloads = useCallback(() => {
    const url = config.updates.downloadUrl;
    if (!url) return;

    let target = url;
    if (hasBearerToken()) {
      try {
        // In dev the URL is relative to the Vite server, so resolve it first.
        const withCredentials = new URL(url, window.location.href);
        withCredentials.username = config.updates.basicUsername;
        withCredentials.password = config.updates.bearerToken;
        // URL#toString percent-encodes the credentials, which nginx decodes
        // back to the raw token before comparing it.
        target = withCredentials.toString();
      } catch (err: any) {
        console.warn(
          "Could not build the authenticated download URL:",
          err.message,
        );
      }
    }

    // Called directly from the click handler so the popup is user-initiated and
    // not blocked. If it is blocked anyway, fall back to a same-tab navigation.
    const opened = window.open(target, "_blank", "noopener");
    if (!opened) {
      window.location.assign(target);
    }
  }, []);

  const test = true;

  return (
    <div className="flex items-center text-xs">
      {newVersionAvailable && (
        <button
          type="button"
          className="font-normal cursor-pointer"
          onClick={openDownloads}
        >{`New Version ${versionNumber} available!`}</button>
      )}
      {!newVersionAvailable && (
        <div title={error ?? undefined}>{`version ${versionNumber}`}</div>
      )}
      {!didUpdateCheck && (
        <div className="w-2.5 h-2.5 border ml-2.5">
          <span className="loading loading-spinner loading-md"></span>
        </div>
      )}
    </div>
  );
};
