import { view } from "@risingstack/react-easy-state";
import React from "react";
import { appState } from "../../../state/appState";
import { GradientColorPicker } from "../../utils/GradientColorPicker";
import { ColorPicker } from "../../utils/ColorPicker";
import { Tools } from "../SidebarRight";
import { Canvas } from "../../canvas/Canvas";
import { Brush, Eraser, PaintBucket, SprayCan, CircleX } from "lucide-react";

import "./SidebarDraw.css";

interface Props {
  getCanvas: () => Canvas;
}

export const SidebarDraw: React.FC<Props> = view(({ getCanvas }) => {
  const renderColorPicker = () =>
    appState.tools.selected === Tools.GRADIENT ? (
      <GradientColorPicker
        onColor1Select={(color) => (appState.tools.gradientColor1 = color)}
        onColor2Select={(color) => (appState.tools.gradientColor2 = color)}
        color1={appState.tools.gradientColor1}
        color2={appState.tools.gradientColor2}
      />
    ) : (
      <ColorPicker />
    );

  return (
    <div className={`sidebar-draw ${appState.tools.selected.toLowerCase()}`}>
      <div className="tool-selection">
        <Brush
          className={`sidebar-icon ${
            appState.tools.selected === Tools.BRUSH ? "is-active" : ""
          }`}
          title={Tools.BRUSH}
          onClick={() => (appState.tools.selected = Tools.BRUSH)}
        />
        <Eraser
          className={`sidebar-icon ${
            appState.tools.selected === Tools.ERASER ? "is-active" : ""
          }`}
          title={Tools.ERASER}
          onClick={() => (appState.tools.selected = Tools.ERASER)}
        />
        <PaintBucket
          className={`sidebar-icon ${
            appState.tools.selected === Tools.FILL ? "is-active" : ""
          }`}
          title={Tools.FILL}
          onClick={() => (appState.tools.selected = Tools.FILL)}
        />
        <SprayCan
          className={`sidebar-icon ${
            appState.tools.selected === Tools.GRADIENT ? "is-active" : ""
          }`}
          title={Tools.GRADIENT}
          onClick={() => (appState.tools.selected = Tools.GRADIENT)}
        />
        <CircleX
          className={`sidebar-icon ${
            appState.tools.selected === Tools.CLEAR ? "is-active" : ""
          }`}
          onClick={() => getCanvas().clear()}
          title="Clear"
        />
      </div>
      <div className="color-picker">{renderColorPicker()}</div>
    </div>
  );
});
