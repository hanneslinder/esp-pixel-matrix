import { view } from "@risingstack/react-easy-state";
import React, { useState } from "react";
import { syncFullStateAction } from "../../../Actions";
import { appState } from "../../../state/appState";
import {
  AppStateToPersist,
  deleteItem,
  SavedItem,
  saveView,
} from "../../../utils/storage";
import { Canvas, PixelData } from "../../canvas/Canvas";

import { Save, CirclePlus, CircleX } from "lucide-react";
// sample some pixels, should be unique enough
function getItemKey(item: SavedItem) {
  return (
    item.id +
    item.pixelData[32 * 5].c +
    item.pixelData[32 * 15].c +
    item.pixelData[15 * 10].c
  );
}

interface SavedItemProps {
  getCanvas: () => Canvas;
}

export const SaveLoad: React.FC<SavedItemProps> = view(({ getCanvas }) => {
  const [saveName, setSaveName] = useState("");

  const save = (name = saveName, id?: string) => {
    saveView(getCanvas().getPixelData(), name, id);
    setSaveName("");
  };

  const loadItem = (item: SavedItem) => {
    getCanvas().setPixels(item.pixelData, false);

    Object.keys(item.state).forEach((k: keyof AppStateToPersist) => {
      console.log(appState[k], item.state[k]);
      (appState[k] as any) = item.state[k];
    });

    appState.loadedItemId = item.id;

    syncFullStateAction(item.pixelData);
  };

  const savedItems = appState.savedItems.sort((a, b) =>
    a.modified < b.modified ? 1 : -1
  );

  return (
    <div>
      <div className="border-b border-[--color-dark-1] mb-5">
        <div className="flex my-5 items-center justify-between">
          <div className="flex-grow mr-5">
            <input
              placeholder="View Name"
              type="text"
              className="bg-[--color-dark-1] text-[--color-gray-0] px-2.5 py-1.5 border-none h-10 rounded-md w-full"
              value={saveName}
              onChange={(e) =>
                setSaveName((e.target as HTMLInputElement).value)
              }
            />
          </div>
          <button
            className="mt-0 flex items-center cursor-pointer bg-[--color-dark-1] text-[--color-gray-0] px-2.5 py-2.5 border-none rounded-md"
            onClick={() => save()}
          >
            <span className="mr-1.5">Save</span>
          </button>
        </div>
        <div className="flex flex-col">
          {savedItems.map((item) => (
            <div
              key={getItemKey(item)}
              className="flex items-center py-2.5 border-t border-[--color-dark-1]"
            >
              <div
                className="w-[100px] whitespace-nowrap w-full overflow-hidden text-ellipsis cursor-pointer"
                onClick={() => loadItem(item)}
              >
                {item.name}
              </div>

              <PreviewCanvas
                pixelData={item.pixelData}
                onClick={() => loadItem(item)}
              />
              <Save
                className="mx-1.5 cursor-pointer [&>svg]:stroke-[1px]"
                title="overwrite"
                onClick={() => save(item.name, item.id)}
              />
              <CircleX
                className="mx-1.5 cursor-pointer [&>svg]:stroke-[1px]"
                title="delete"
                onClick={() => deleteItem(item.id)}
              />
            </div>
          ))}
        </div>
      </div>
    </div>
  );
});

interface PreviewCanvasProps {
  pixelData: PixelData[];
  onClick: () => void;
}

class PreviewCanvas extends React.Component<PreviewCanvasProps, {}> {
  private canvasRef: HTMLCanvasElement;
  private ctx: CanvasRenderingContext2D;

  componentDidMount() {
    this.ctx = this.canvasRef.getContext("2d");
    this.canvasRef.width = appState.matrix.width;
    this.canvasRef.height = appState.matrix.height;
    this.props.pixelData.forEach((p) => this.drawPixel(p.p[0], p.p[1], p.c));
  }

  private drawPixel(x: number, y: number, color: string): void {
    this.ctx.fillStyle = color;
    this.ctx.fillRect(x, y, 1, 1);
  }

  render() {
    return (
      <canvas
        ref={(r) => {
          this.canvasRef = r;
        }}
        onClick={this.props.onClick}
        className="border border-[--color-dark-1] mr-5 ml-5 cursor-pointer transition-[border-color] duration-300 hover:border-[--color-gray-0]"
      />
    );
  }
}
