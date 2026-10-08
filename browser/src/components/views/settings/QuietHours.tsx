import { view } from "@risingstack/react-easy-state";
import { Moon, X } from "lucide-react";
import React from "react";
import { clearQuietHoursAction, setQuietHoursAction } from "../../../Actions";
import { appState } from "../../../state/appState";

interface Props {}

/**
 * Turns any value coming out of <input type="time"> into the "HH:MM" the
 * firmware expects. Browsers may include seconds ("22:00:00") and may return
 * an empty string while the user is still typing.
 */
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
  const isSet = Boolean(quietHoursStart || quietHoursEnd);

  const onChange = (part: "start" | "end", value: string) => {
    const normalized = toHoursMinutes(value);

    // The firmware treats a half filled window as disabled, so sending one
    // bound before the other is set is harmless.
    if (part === "start") {
      setQuietHoursAction(normalized, quietHoursEnd);
    } else {
      setQuietHoursAction(quietHoursStart, normalized);
    }
  };

  return (
    <div className="border-b border-gray-700 pb-5 mb-5">
      <div className="mb-2 flex items-center gap-2">
        <span>Quiet hours</span>
        {isSet && (
          <span className="text-xs opacity-60">{`${quietHoursStart} - ${quietHoursEnd}`}</span>
        )}
      </div>

      <div className="flex items-center gap-2">
        <div className="w-8 flex items-center justify-center">
          <Moon />
        </div>

        <input
          type="time"
          value={quietHoursStart}
          onChange={(e) => onChange("start", e.target.value)}
          aria-label="Quiet hours start"
          className="input input-sm bg-gray-900 w-28 text-center"
        />

        <span className="opacity-60">-</span>

        <input
          type="time"
          value={quietHoursEnd}
          onChange={(e) => onChange("end", e.target.value)}
          aria-label="Quiet hours end"
          className="input input-sm bg-gray-900 w-28 text-center"
        />

        {isSet && (
          <button
            type="button"
            onClick={() => clearQuietHoursAction()}
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
