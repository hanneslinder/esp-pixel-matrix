import { store } from "@risingstack/react-easy-state";
import { Views } from "../components/Navigation";
import { Tools } from "../components/views/ViewWrapper";
import { getSavedItemsFromLocalStorage, SavedItem } from "../utils/storage";

export interface Settings {
  brightness: number;
  compositionMode: number;
  locale: string;
  width: number;
  height: number;
  pixelRatio: number;
}

export interface AppState {
  view: Views;
  tools: {
    selected: Tools;
    color: string;
    gradientColor1: string;
    gradientColor2: string;
    brushSize?: number;
  };
  settings: Settings;
  customData: CustomDataOptions;
  text: TextOptions[];
  connection: {
    isSending: boolean;
    isReceiving: boolean;
    state: number;
    updatePending: boolean;
    updatePercent?: number;
  };
  savedItems: SavedItem[];
  loadedItemId?: string;
}

export enum TextAlign {
  LEFT,
  CENTER,
  RIGHT,
}

export enum Font {
  REGULAR,
  PICO,
}

export interface TextOptions {
  color: string;
  text: string;
  line: number;
  align: TextAlign;
  offsetX: number;
  offsetY: number;
  size: number;
  font: Font;
}

export interface CustomDataOptions {
  updateInterval: number;
  server: string;
}

const getInitialState = (): AppState => ({
  view: Views.Draw,
  tools: {
    selected: Tools.BRUSH,
    color: "#ffffff",
    gradientColor1: "#2db2eb",
    gradientColor2: "#1d59b9",
    brushSize: 1,
  },
  connection: {
    isSending: false,
    isReceiving: false,
    state: 0,
    updatePending: false,
  },
  text: [
    {
      color: "#ffffff",
      text: "%H:%M",
      size: 2,
      line: 1,
      align: TextAlign.CENTER,
      offsetX: -1,
      offsetY: -5,
      font: Font.REGULAR,
    },
    {
      color: "#ffffff",
      text: "%d.%b",
      size: 1,
      line: 2,
      align: TextAlign.CENTER,
      offsetX: 2,
      offsetY: -3,
      font: Font.REGULAR,
    },
  ],
  settings: {
    compositionMode: 0,
    brightness: 2,
    locale: "en_US.UTF-8",
    width: 64,
    height: 32,
    pixelRatio: 10,
  },
  customData: {
    updateInterval: -1,
    server: "",
  },
  savedItems: getSavedItemsFromLocalStorage(),
});

export const appState: AppState = store(getInitialState());
