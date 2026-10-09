import { appState, TextOptions } from "./appState";

export interface StateFromRemote {
  text?: TextOptions[];
  customData: boolean;
  customDataServer: string;
  customDataInterval: number;
  compositionMode: number;
  brightness: number;
  timezone: string;
  locale: string;
  quietHoursStart: string;
  quietHoursEnd: string;
}

export const applyRemoteState = (state: StateFromRemote): void => {
  if (Array.isArray(state.text)) {
    appState.text = state.text;
  }

  if (typeof state.brightness === "number") {
    appState.settings.brightness = state.brightness;
  }

  if (typeof state.compositionMode === "number") {
    appState.settings.compositionMode = state.compositionMode;
  }

  if (typeof state.locale === "string") {
    appState.settings.locale = state.locale;
  }

  if (typeof state.quietHoursStart === "string") {
    appState.settings.quietHoursStart = state.quietHoursStart;
  }

  if (typeof state.quietHoursEnd === "string") {
    appState.settings.quietHoursEnd = state.quietHoursEnd;
  }

  if (typeof state.customData === "boolean") {
    appState.customData = {
      updateInterval: state.customData ? state.customDataInterval : -1,
      server: state.customDataServer ?? "",
    };
  }
};
