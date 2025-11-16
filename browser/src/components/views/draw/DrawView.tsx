import { view } from "@risingstack/react-easy-state";
import React from "react";
import { appState } from "../../../state/appState";
import { GradientColorPicker } from "../../utils/GradientColorPicker";
import { ColorPicker } from "../../utils/ColorPicker";
import { Tools } from "../ViewWrapper";
import { Canvas } from "../../canvas/Canvas";
import { Brush, Eraser, PaintBucket, Diameter, Trash2 } from "lucide-react";
import { clsx } from "clsx";

interface Props {
  getCanvas: () => Canvas;
}

export const DrawView: React.FC<Props> = view(({ getCanvas }) => {
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
      <div className="tooltip" data-tip={tool}>
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
      </div>
    );
  };

  return (
    <div>
      <div className="flex justify-around">
        {renderIcon(Tools.BRUSH, Brush)}
        {renderIcon(Tools.ERASER, Eraser)}
        {renderIcon(Tools.FILL, PaintBucket)}
        {renderIcon(Tools.GRADIENT, Diameter)}
        <div
          className="cursor-pointer p-2 flex items-center justify-center hover:bg-gray-700 rounded-md"
          onClick={() => getCanvas().clear()}
          title="Clear"
        >
          <Trash2 />
        </div>
      </div>
      <div className="mt-4 text-center">{appState.tools.selected}</div>
      <div
        className={clsx(
          "flex justify-center mt-4 color-picker",
          appState.tools.selected === Tools.ERASER &&
            "opacity-25 pointer-events-none"
        )}
      >
        {renderColorPicker()}
      </div>
      <div>
        {appState.tools.selected === Tools.BRUSH && (
          <div className="border-b border-gray-700 p-5">
            <div className="mb-2">Brush Size</div>
            <div className="flex items-center">
              <input
                type="range"
                min="1"
                max="5"
                step="1.0"
                value={appState.tools.brushSize}
                onChange={(e) =>
                  (appState.tools.brushSize = parseInt(
                    (e.target as HTMLInputElement).value
                  ))
                }
                className="range range-sm flex-grow mr-5 my-0"
              />
              <div className="w-8">{appState.tools.brushSize}</div>
            </div>
          </div>
        )}
      </div>
    </div>
  );
});
