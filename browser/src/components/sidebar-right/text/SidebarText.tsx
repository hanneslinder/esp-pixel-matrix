import { view } from "@risingstack/react-easy-state";
import React, { useRef } from "react";
import { setCompositionModeAction, setTextAction } from "../../../Actions";
import {
  appState,
  Font,
  TextAlign,
  TextOptions,
} from "../../../state/appState";
import { TextOptionsControl } from "./TextOptionsControl";
import { Expandable } from "../../utils/Expandable";
import { Blend, CirclePlus } from "lucide-react";

export const SidebarText = view(() => {
  const setCompositionMode = () => {
    const currentMode = appState.settings.compositionMode;
    let newMode = currentMode + 1;
    setCompositionModeAction(newMode);
  };

  const renderTextSettings = () => {
    return appState.text.map((text, i) => (
      <Expandable
        initialOpen={true}
        collapsedContent={<div>Text Line {i + 1}</div>}
        expandedContent={<TextOptionsControl settings={text} />}
        key={`${i}-${text.line}`}
      />
    ));
  };

  const addTextItem = () => {
    const textItem: TextOptions = {
      align: TextAlign.CENTER,
      color: "#ffffff",
      text: "",
      size: 1,
      line: appState.text.length + 1,
      offsetX: 3,
      offsetY: 1,
      font: Font.REGULAR,
    };

    appState.text.push(textItem);
    setTextAction();
  };

  const renderAddTextButton = () => {
    if (appState.text.length < 5) {
      return (
        <div className="flex justify-center my-5 mb-12">
          <button className="btn btn-sm btn-primary" onClick={addTextItem}>
            <CirclePlus className="mr-2" style={{ strokeWidth: 2 }} size={18} />
            Add Text
          </button>
        </div>
      );
    }
  };

  return (
    <div className="mx-5">
      <div className="mb-5">
        <div className="flex my-5 items-center">
          <div className="flex-grow items-center flex">Blend Mode</div>
          <div className="flex-grow-0 flex items-center">
            <Blend className="cursor-pointer" onClick={setCompositionMode} />
          </div>
        </div>
      </div>
      {renderTextSettings()}
      {renderAddTextButton()}
    </div>
  );
});
