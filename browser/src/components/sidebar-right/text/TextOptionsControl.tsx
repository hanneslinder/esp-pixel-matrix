import { view } from "@risingstack/react-easy-state";
import React from "react";
import { HexColorPicker } from "react-colorful";
import { setTextAction } from "../../../Actions";
import { limitNumberBetween } from "../../../utils/utils";
import { Expandable } from "../../utils/Expandable";

import { MessageCircleQuestionMark, CircleX } from "lucide-react";
import { DropdownItem, Dropdown } from "../../utils/Dropdown";
import {
  appState,
  Font,
  TextAlign,
  TextOptions,
} from "../../../state/appState";

const getTextAlignItems = (): DropdownItem[] => [
  { label: "Left", value: TextAlign.LEFT },
  { label: "Center", value: TextAlign.CENTER },
  { label: "Right", value: TextAlign.RIGHT },
];

const getFontItems = (): DropdownItem[] => [
  { label: "Regular", value: Font.REGULAR },
  { label: "Pico", value: Font.PICO },
];

interface Props {
  settings: TextOptions;
  onResize?: () => void;
}

let colorChangeTimeout: ReturnType<typeof setTimeout>;
const COLORPICKER_TIMEOUT_MS = 350;

export const TextOptionsControl: React.FC<Props> = view(
  ({ settings, onResize }) => {
    const onTextChange = (text: string) => {
      const newSettings = { ...settings, text };
      updateTextItem(newSettings);
    };

    const onXOffsetChange = (offset: number) => {
      const offsetX = limitNumberBetween(offset, -20, 20);
      const newSettings = { ...settings, offsetX };
      updateTextItem(newSettings);
    };

    const onYOffsetChange = (offset: number) => {
      const offsetY = limitNumberBetween(offset, -20, 20);
      const newSettings = { ...settings, offsetY };
      updateTextItem(newSettings);
    };

    const changeSize = (size: number) => {
      const newSettings = { ...settings, size };
      updateTextItem(newSettings);
    };

    const onFontChange = (font: Font) => {
      const newSettings = { ...settings, font };
      updateTextItem(newSettings);
    };

    const onAlignChange = (align: TextAlign) => {
      const newSettings = { ...settings, align };
      updateTextItem(newSettings);
    };

    const onColorChange = (color: string) => {
      clearTimeout(colorChangeTimeout);
      colorChangeTimeout = setTimeout(() => {
        const newSettings = { ...settings, color };
        updateTextItem(newSettings);
      }, COLORPICKER_TIMEOUT_MS);
    };

    const removeTextItem = (line: number) => {
      appState.text = appState.text.filter((t) => t.line !== line);
      setTextAction();
    };

    const updateTextItem = (newText: TextOptions) => {
      const index = appState.text.findIndex((t) => t.line === newText.line);

      if (index > -1) {
        appState.text[index] = newText;
        setTextAction();
      }
    };

    const updateHeightOnAnimationEnd = () => {
      setTimeout(() => {
        onResize();
      }, COLORPICKER_TIMEOUT_MS);
    };

    const clorpickerPreview = (
      <div
        className="w-10 h-5 mr-0 rounded-md transition-[margin] duration-[600ms] color-picker-preview"
        style={{ background: settings.color }}
      />
    );

    console.log(settings);

    return (
      <div className="border-b border-[--color-dark-1] mb-5">
        <div className="flex my-5 items-center border-b border-[--color-dark-1] pb-5">
          <div className="flex-grow items-center flex">
            <span>Text</span>
            <MessageCircleQuestionMark
              className="ml-2.5 cursor-pointer transition-opacity duration-300 hover:opacity-100"
              title="help"
              size={16}
              onClick={() =>
                window.open(
                  "https://www.cplusplus.com/reference/ctime/strftime/",
                  "_blank"
                )
              }
            />
          </div>
          <div className="flex-grow-0 flex items-center">
            <input
              type="text"
              className="bg-[--color-dark-1] text-[--color-gray-0] px-2.5 py-1.5 border-none h-[30px] rounded-md"
              value={settings.text}
              onChange={(e) =>
                onTextChange((e.target as HTMLInputElement).value)
              }
            />
          </div>
        </div>
        <div className="flex my-5 items-center">
          <div className="flex-grow items-center flex">Font</div>
          <div className="flex-grow-0 flex items-center">
            <Dropdown
              selected={settings.font}
              items={getFontItems()}
              onSelect={onFontChange}
            />
          </div>
        </div>
        <div className="flex my-5 items-center">
          <div className="flex-grow items-center flex">Size</div>
          <div className="flex-grow-0 flex items-center">
            <div>{settings.size}</div>
            <div className="flex-grow-0 basis-10 flex ml-5">
              <div
                className={`flex-grow-0 w-[30px] h-[30px] select-none flex justify-center content-center bg-[--color-dark-1] cursor-pointer items-center rounded-tl-sm rounded-bl-sm border-r border-[--color-gray-1] hover:bg-[--color-blue-0] ${
                  settings.size === 1 ? "opacity-50 pointer-events-none" : ""
                }`}
                onClick={() => changeSize(settings.size - 1)}
              >
                -
              </div>
              <div
                className={`flex-grow-0 w-[30px] h-[30px] select-none flex justify-center content-center bg-[--color-dark-1] cursor-pointer items-center rounded-tr-sm rounded-br-sm hover:bg-[--color-blue-0] ${
                  settings.size > 4 ? "opacity-50 pointer-events-none" : ""
                }`}
                onClick={() => changeSize(settings.size + 1)}
              >
                +
              </div>
            </div>
          </div>
        </div>
        <div className="flex my-5 items-center">
          <div className="flex-grow items-center flex">Align</div>
          <div className="flex-grow-0 flex items-center">
            <Dropdown
              selected={settings.align}
              items={getTextAlignItems()}
              onSelect={onAlignChange}
            />
          </div>
        </div>
        <div className="flex my-5 items-center">
          <div className="flex-grow items-center flex">Offset X</div>
          <div className="flex-grow-0 flex items-center">
            <div>{settings.offsetX}</div>
            <div className="flex-grow-0 basis-10 flex ml-5">
              <div
                className={`flex-grow-0 w-[30px] h-[30px] select-none flex justify-center content-center bg-[--color-dark-1] cursor-pointer items-center rounded-tl-sm rounded-bl-sm border-r border-[--color-gray-1] hover:bg-[--color-blue-0] ${
                  settings.offsetX < -19 ? "opacity-50 pointer-events-none" : ""
                }`}
                onClick={() => onXOffsetChange(settings.offsetX - 1)}
              >
                -
              </div>
              <div
                className={`flex-grow-0 w-[30px] h-[30px] select-none flex justify-center content-center bg-[--color-dark-1] cursor-pointer items-center rounded-tr-sm rounded-br-sm hover:bg-[--color-blue-0] ${
                  settings.offsetX > 19 ? "opacity-50 pointer-events-none" : ""
                }`}
                onClick={() => onXOffsetChange(settings.offsetX + 1)}
              >
                +
              </div>
            </div>
          </div>
        </div>
        <div className="flex my-5 items-center">
          <div className="flex-grow items-center flex">Offset Y</div>
          <div className="flex-grow-0 flex items-center">
            <div>{settings.offsetY}</div>
            <div className="flex-grow-0 basis-10 flex ml-5">
              <div
                className={`flex-grow-0 w-[30px] h-[30px] select-none flex justify-center content-center bg-[--color-dark-1] cursor-pointer items-center rounded-tl-sm rounded-bl-sm border-r border-[--color-gray-1] hover:bg-[--color-blue-0] ${
                  settings.offsetY < -19 ? "opacity-50 pointer-events-none" : ""
                }`}
                onClick={() => onYOffsetChange(settings.offsetY - 1)}
              >
                -
              </div>
              <div
                className={`flex-grow-0 w-[30px] h-[30px] select-none flex justify-center content-center bg-[--color-dark-1] cursor-pointer items-center rounded-tr-sm rounded-br-sm hover:bg-[--color-blue-0] ${
                  settings.offsetY > 19 ? "opacity-50 pointer-events-none" : ""
                }`}
                onClick={() => onYOffsetChange(settings.offsetY + 1)}
              >
                +
              </div>
            </div>
          </div>
        </div>
        <div className="flex my-5 items-center border-b border-[--color-dark-1] pb-5">
          <Expandable
            title="Color"
            initialOpen={false}
            titleElement={clorpickerPreview}
            onExpand={updateHeightOnAnimationEnd}
            className="[&_.expandable-icon]:w-0 [&_.expandable-icon]:overflow-hidden [&_.expandable-icon]:transition-[width] [&_.expandable-icon]:duration-[600ms] [&.is-expanded_.expandable-icon]:w-5 [&.is-expanded_.color-picker-preview]:mr-5"
          >
            <div className="p-5">
              <HexColorPicker
                color={settings.color}
                onChange={(color) => onColorChange(color)}
              />
            </div>
          </Expandable>
        </div>
        <div className="flex justify-end mb-5 -mt-[30px]">
          <button
            className="cursor-pointer bg-[--color-dark-1] text-[--color-gray-0] px-2.5 py-2.5 mt-2.5 block border-none rounded-md"
            onClick={() => removeTextItem(settings.line)}
          >
            delete
          </button>
        </div>
      </div>
    );
  }
);
