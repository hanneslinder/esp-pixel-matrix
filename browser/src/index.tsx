import React, { useRef } from "react";
import ReactDOM from "react-dom";
import { view } from "@risingstack/react-easy-state";

import {
  CanvasWrapper,
  CanvasWrapperElement,
} from "./components/canvas/CanvasWrapper";
import { Header } from "./components/Header";
import { ViewWrapper } from "./components/views/ViewWrapper";
import { Navigation } from "./components/Navigation";

import "./index.css";

const App = view(() => {
  const canvasWrapperRef = useRef<CanvasWrapperElement>(null);

  return (
    <div className="flex flex-col flex-grow w-screen bg-gray-900 font-sans text-sm">
      <Header />
      <div className="flex justify-between h-[calc(100vh-40px)]">
        <div className="bg-gray-800 max-h-[calc(100vh-40px)] overflow-visible">
          <Navigation />
        </div>
        <div className="flex-grow flex-shrink overflow-auto flex items-center flex-col justify-center">
          <CanvasWrapper ref={canvasWrapperRef} />
        </div>
        <div className="bg-gray-800 flex-grow-0 basis-[350px] min-w-[280px] max-h-[calc(100vh-40px)] overflow-auto">
          <ViewWrapper getCanvas={() => canvasWrapperRef.current.canvas} />
        </div>
      </div>
    </div>
  );
});

ReactDOM.render(<App />, document.getElementById("app"));
