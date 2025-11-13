import { view } from "@risingstack/react-easy-state";
import React from "react";
import { appState } from "../../../state/appState";
import { GradientColorPicker } from "../../utils/GradientColorPicker";
import { ColorPicker } from "../../utils/ColorPicker";
import { Tools } from "../SidebarRight";
import { Canvas } from "../../canvas/Canvas";
import { Brush, Eraser, PaintBucket, SprayCan, CircleX } from "lucide-react";

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
    <div
      className={
        appState.tools.selected === Tools.ERASER
          ? "[&_.color-picker]:opacity-25 [&_.color-picker]:pointer-events-none"
          : ""
      }
    >
      <div className="flex justify-around border-t border-b border-[--color-dark-3] [&>svg]:flex-grow [&>svg]:p-2.5">
        <Brush
          className={`cursor-pointer flex flex-col items-center justify-center [&>svg]:stroke-[1px] ${
            appState.tools.selected === Tools.BRUSH
              ? "bg-[--color-highlight-1]"
              : "hover:bg-[--color-dark-3]"
          }`}
          title={Tools.BRUSH}
          onClick={() => (appState.tools.selected = Tools.BRUSH)}
        />
        <Eraser
          className={`cursor-pointer flex flex-col items-center justify-center [&>svg]:stroke-[1px] ${
            appState.tools.selected === Tools.ERASER
              ? "bg-[--color-highlight-1]"
              : "hover:bg-[--color-dark-3]"
          }`}
          title={Tools.ERASER}
          onClick={() => (appState.tools.selected = Tools.ERASER)}
        />
        <PaintBucket
          className={`cursor-pointer flex flex-col items-center justify-center [&>svg]:stroke-[1px] ${
            appState.tools.selected === Tools.FILL
              ? "bg-[--color-highlight-1]"
              : "hover:bg-[--color-dark-3]"
          }`}
          title={Tools.FILL}
          onClick={() => (appState.tools.selected = Tools.FILL)}
        />
        <SprayCan
          className={`cursor-pointer flex flex-col items-center justify-center [&>svg]:stroke-[1px] ${
            appState.tools.selected === Tools.GRADIENT
              ? "bg-[--color-highlight-1]"
              : "hover:bg-[--color-dark-3]"
          }`}
          title={Tools.GRADIENT}
          onClick={() => (appState.tools.selected = Tools.GRADIENT)}
        />
        <CircleX
          className="cursor-pointer flex flex-col items-center justify-center [&>svg]:stroke-[1px] hover:bg-[--color-dark-3]"
          onClick={() => getCanvas().clear()}
          title="Clear"
        />
      </div>
      <div className="flex justify-center py-12 border-b border-[--color-dark-3] color-picker">
        {renderColorPicker()}
      </div>
    </div>
  );
});
