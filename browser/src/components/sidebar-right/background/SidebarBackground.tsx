import { view } from "@risingstack/react-easy-state";
import React from "react";
import { Canvas } from "../../canvas/Canvas";
import { FilePicker } from "../../utils/FilePicker";
import { RandomImageList } from "./RandomImageList";

interface Props {
  getCanvas: () => Canvas;
}

const isImage = (mimeType: string) => mimeType.startsWith("image/");

export const SidebarBackground: React.FC<Props> = view(({ getCanvas }) => {
  const drawImage = (img: HTMLImageElement) => {
    img.onload = () => {
      const canvas = getCanvas();
      canvas.drawImage(img);
    };
  };
  const handleImageUpload = (file: File) => {
    const img = new Image();
    img.setAttribute("src", URL.createObjectURL(file));
    drawImage(img);
  };

  const handleRandomImage = (url: string) => {
    const img = new Image();
    img.crossOrigin = "Anonymous";
    img.setAttribute("src", url);
    drawImage(img);
  };

  return (
    <div className="flex flex-col mx-5 h-[calc(100vh-40px)]">
      <FilePicker
        onFileDroppedOrSelected={handleImageUpload}
        isFileTypeAllowed={isImage}
        label="Drag and drop image here"
      />
      <div className="flex flex-grow basis-[200px] flex-col mt-12 overflow-hidden">
        <RandomImageList imageSelected={handleRandomImage} />
      </div>
    </div>
  );
});
