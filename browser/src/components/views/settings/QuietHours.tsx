import { view } from "@risingstack/react-easy-state";
import { Moon, X } from "lucide-react";
import React, { useEffect, useMemo, useState } from "react";
import { clearQuietHoursAction, setQuietHoursAction } from "../../../Actions";
import { appState } from "../../../state/appState";
import { debounce } from "../../../utils/utils";

interface Props {}

const COMMIT_DELAY_MS = 600;

interface QuietHoursWindow {
  start: string;
  end: string;
}

const toHoursMinutes = (value: string): string => {
  if (!value) {
    return "";
  }

  const [hours, minutes] = value.split(":");
  if (hours === undefined || minutes === undefined) {
    return "";
  }

  return `${hours.padStart(2, "0")}:${minutes.padStart(2, "0")}`;
};

export const QuietHours: React.FC<Props> = view(() => {
  const { quietHoursStart, quietHoursEnd } = appState.settings;

  // Do not sync matrixSettings while editing quiet hours.
  // Otherwise the inputs would be overwritten by the echoed state from the device.
  const [draft, setDraft] = useState<QuietHoursWindow | null>(null);
  const { start, end } = draft ?? {
    start: quietHoursStart,
    end: quietHoursEnd,
  };

  const commit = useMemo(
    () =>
      debounce((start: string, end: string) => {
        // Wait until both start and end are either set or empty before committing.
        if ((start === "") !== (end === "")) {
          return;
        }

        setQuietHoursAction(start, end);
        setDraft(null);
      }, COMMIT_DELAY_MS),
    [],
  );

  useEffect(() => () => commit.flush(), [commit]);

  const onChange = (part: "start" | "end", value: string) => {
    const normalized = toHoursMinutes(value);
    const next =
      part === "start"
        ? { start: normalized, end }
        : { start, end: normalized };

    setDraft(next);
    commit(next.start, next.end);
  };

  const clear = () => {
    commit.cancel();
    setDraft(null);
    clearQuietHoursAction();
  };

  const isSet = Boolean(start || end);

  return (
    <div className="border-b border-gray-700 pb-5 mb-5">
      <div className="mb-2 flex items-center gap-2">
        <span>Quiet hours</span>
        {isSet && (
          <span className="text-xs opacity-60">{`${start} - ${end}`}</span>
        )}
      </div>

      <div className="flex items-center gap-2">
        <div className="w-8 flex items-center justify-center">
          <Moon />
        </div>

        <input
          type="time"
          value={start}
          onChange={(e) => onChange("start", e.currentTarget.value)}
          aria-label="Quiet hours start"
          className="input input-sm bg-gray-900 w-28 text-center"
        />

        <span className="opacity-60">-</span>

        <input
          type="time"
          value={end}
          onChange={(e) => onChange("end", e.currentTarget.value)}
          aria-label="Quiet hours end"
          className="input input-sm bg-gray-900 w-28 text-center"
        />

        {isSet && (
          <button
            type="button"
            onClick={clear}
            aria-label="Delete quiet hours"
            title="Delete quiet hours"
            className="btn btn-sm btn-ghost btn-square"
          >
            <X size={16} />
          </button>
        )}
      </div>
    </div>
  );
});
