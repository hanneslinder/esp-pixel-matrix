import { view } from "@risingstack/react-easy-state";
import React from "react";
import { appState } from "../../../state/appState";
import { GradientColorPicker } from "../../utils/GradientColorPicker";
import { ColorPicker } from "../../utils/ColorPicker";
import { Tools } from "../SidebarRight";
import { Canvas } from "../../canvas/Canvas";
import { Brush, Eraser, PaintBucket, SprayCan, CircleX } from "lucide-react";
import { clsx } from "clsx";

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

  const renderIcon = (
    tool: Tools,
    IconComponent: React.FC<React.SVGProps<SVGSVGElement>>
  ) => {
    return (
      <div
        onClick={() => (appState.tools.selected = tool)}
        title={tool}
        className={clsx(
          "cursor-pointer p-2 flex items-center justify-center rounded-md ",
          appState.tools.selected === tool
            ? "bg-gray-900 !hover:bg-gray-900"
            : "hover:bg-gray-700"
        )}
      >
        <IconComponent />
      </div>
    );
  };

  return (
    <div>
      <div className="flex justify-around">
        {renderIcon(Tools.BRUSH, Brush)}
        {renderIcon(Tools.ERASER, Eraser)}
        {renderIcon(Tools.FILL, PaintBucket)}
        {renderIcon(Tools.GRADIENT, SprayCan)}
        <div
          className="cursor-pointer p-2 flex items-center justify-center hover:bg-gray-700 rounded-md"
          onClick={() => getCanvas().clear()}
          title="Clear"
        >
          <CircleX />
        </div>
      </div>
      <div
        className={clsx(
          "flex justify-center py-12 color-picker",
          appState.tools.selected === Tools.ERASER &&
            "opacity-25 pointer-events-none"
        )}
      >
        {renderColorPicker()}
      </div>
      <div>
        {appState.tools.selected === Tools.BRUSH && (
          <div className="flex justify-center items-center gap-5">
            <label htmlFor="brush-size">Brush Size:</label>
            <input
              className="range w-32"
              value={appState.tools.brushSize}
              onChange={(e) =>
                (appState.tools.brushSize = parseInt(
                  (e.target as HTMLInputElement).value
                ))
              }
              type="range"
              min={1}
              max={5}
            />
          </div>
        )}
      </div>
    </div>
  );
});
