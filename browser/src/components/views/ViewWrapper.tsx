import { view } from "@risingstack/react-easy-state";
import React, { useCallback } from "react";
import { appState } from "../../state/appState";
import { Canvas } from "../canvas/Canvas";
import { Views } from "../Navigation";
import { BackgroundView } from "./background/BackgroundView";
import { TextView } from "./text/TextView";
import { SettingsView } from "./settings/SettingsView";
import { DrawView } from "./draw/DrawView";

export const enum Tools {
  BRUSH = "Brush",
  ERASER = "Eraser",
  FILL = "Fill",
  CLEAR = "Clear",
  GRADIENT = "Gradient",
}

interface Props {
  getCanvas: () => Canvas;
}

export const ViewWrapper: React.FC<Props> = view(({ getCanvas }) => {
  const renderActiveView = useCallback(
    (selected: Views) => {
      switch (selected) {
        case Views.Draw:
          return <DrawView getCanvas={getCanvas} />;
        case Views.Text:
          return <TextView />;
        case Views.Background:
          return <BackgroundView getCanvas={getCanvas} />;
        case Views.Settings:
          return <SettingsView getCanvas={getCanvas} />;
      }
    },
    [getCanvas]
  );

  return <div className="flex flex-col">{renderActiveView(appState.view)}</div>;
});
