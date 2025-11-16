import React from "react";
import { view } from "@risingstack/react-easy-state";

import { Canvas, PixelData } from "./Canvas";
import { getSocket, IncommingMessageType } from "../../Websocket";
import { appState } from "../../state/appState";
import { PixelsFromRemote } from "../../utils/storage";

interface CanvasWrapperProps {}
export class CanvasWrapperElement extends React.Component<
  CanvasWrapperProps,
  {}
> {
  private canvasWrapperRef = React.createRef<HTMLDivElement>();
  public canvas: Canvas;
  private unsubcribeFromMatrixPixelMessage;

  constructor(props: CanvasWrapperProps) {
    super(props);

    const socket = getSocket();
    this.unsubcribeFromMatrixPixelMessage = socket.subscribe(
      IncommingMessageType.MatrixPixelResponse,
      this.onMatrixPixelResponse
    );
  }

  public componentWillUnmount() {
    this.unsubcribeFromMatrixPixelMessage();
  }

  private readonly onMatrixPixelResponse = (data: PixelsFromRemote) => {
    if (data["line-start"] === 0) {
      appState.connection.isReceiving = true;
    }

    if (data["line-end"] === appState.settings.height) {
      appState.connection.isReceiving = false;
    }

    let currentLine = data["line-start"];
    data.data.forEach((line) => {
      console.log("Draw pixels in line", currentLine);
      const pixelData: PixelData[] = line.map((d: string, i: number) => ({
        c: d === "#0" ? "#000000" : d,
        p: [i, currentLine],
      }));

      this.canvas.setPixels(pixelData, false);

      currentLine++;
    });
  };

  public componentDidMount() {
    const wrapper = this.canvasWrapperRef.current;
    this.canvas = new Canvas(wrapper);
  }

  private readonly clearLocal = () => {
    this.canvas.clear(false);
  };

  public render() {
    return (
      <div
        id="canvas-wrapper"
        className="relative border-10 border-gray-800 rounded-lg"
        ref={this.canvasWrapperRef}
      />
    );
  }
}

export const CanvasWrapper = view(CanvasWrapperElement);
