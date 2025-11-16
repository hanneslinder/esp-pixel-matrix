import { view } from "@risingstack/react-easy-state";
import React from "react";
import { setLocaleAction } from "../../../Actions";
import { appState } from "../../../state/appState";

interface LocaleProps {}

const localeOptionsMap: { [key: string]: string } = {
  en_US: "en_US.UTF-8",
  de_DE: "de_DE.UTF-8",
};

export const Locale: React.FC<LocaleProps> = view(() => {
  const onLocaleChanged = (selectedLocale: string) => {
    setLocaleAction(localeOptionsMap[selectedLocale]);
  };

  return (
    <div className="settings-item locale">
      <div className="settings-item-label">Locale</div>
      <div className="settings-item-value">
        <select
          defaultValue="Locale"
          className="select"
          onChange={(e) => onLocaleChanged(e.currentTarget.value)}
        >
          <option>en_US</option>
          <option>de_DE</option>
        </select>
      </div>
    </div>
  );
});
