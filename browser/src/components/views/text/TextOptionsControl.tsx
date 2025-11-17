import { view } from "@risingstack/react-easy-state";
import { HexColorPicker } from "react-colorful";
import { setTextAction } from "../../../Actions";
import { clampValue } from "../../../utils/utils";
import { Expandable } from "../../utils/Expandable";

import { MessageCircleQuestionMark } from "lucide-react";
import {
  appState,
  Font,
  TextAlign,
  TextOptions,
} from "../../../state/appState";
import { NumberInput } from "./NumberInput";
import { useState } from "preact/compat";

let colorChangeTimeout: ReturnType<typeof setTimeout>;
const COLORPICKER_TIMEOUT_MS = 350;

interface TextOptionsControlProps {
  settings: TextOptions;
}

export const TextOptionsControl = view(
  ({ settings }: TextOptionsControlProps) => {
    const [colorpickerOpen, setColorpickerOpen] = useState(false);

    const onTextChange = (text: string) => {
      const newSettings = { ...settings, text };
      updateTextItem(newSettings);
    };

    const onXOffsetChange = (offset: number) => {
      const offsetX = clampValue(offset, -20, 20);
      const newSettings = { ...settings, offsetX };
      updateTextItem(newSettings);
    };

    const onYOffsetChange = (offset: number) => {
      const offsetY = clampValue(offset, -20, 20);
      const newSettings = { ...settings, offsetY };
      updateTextItem(newSettings);
    };

    const onSizeChange = (size: number) => {
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
      setColorpickerOpen(true);
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

    const colorpickerPreview = (
      <div
        className="w-10 h-5 mr-0 rounded-md transition-[margin] duration-[600ms] color-picker-preview"
        style={{ background: settings.color }}
      />
    );

    return (
      <div className="flex flex-col gap-2 pr-1">
        <div className="flex items-center pt-1 justify-between">
          <div className="flex-grow items-center flex">
            <span>Text</span>
            <MessageCircleQuestionMark
              className="ml-2.5 cursor-pointer transition-opacity opacity-50 duration-300 hover:opacity-100"
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
              className="input bg-gray-900"
              value={settings.text}
              onChange={(e) =>
                onTextChange((e.target as HTMLInputElement).value)
              }
            />
          </div>
        </div>
        <div className="flex items-center">
          <div className="flex-grow items-center flex">Font</div>
          <div className="flex-grow-0 flex items-center">
            <select
              defaultValue={Font.REGULAR}
              className="select bg-gray-900"
              onChange={(e) => onFontChange(parseInt(e.currentTarget.value))}
            >
              <option value={Font.REGULAR}>Regular</option>
              <option value={Font.PICO}>Pico</option>
            </select>
          </div>
        </div>
        <div className="flex items-center">
          <NumberInput
            label="Size"
            value={settings.size}
            onChange={onSizeChange}
            min={1}
            max={4}
            step={1}
          />
        </div>
        <div className="flex items-center">
          <div className="flex-grow items-center flex">Align</div>
          <div className="flex-grow-0 flex items-center">
            <select
              defaultValue={TextAlign.CENTER}
              className="select bg-gray-900"
              onChange={(e) => onAlignChange(parseInt(e.currentTarget.value))}
            >
              <option value={TextAlign.LEFT}>Left</option>
              <option value={TextAlign.CENTER}>Center</option>
              <option value={TextAlign.RIGHT}>Right</option>
            </select>
          </div>
        </div>
        <div className="flex items-center">
          <NumberInput
            label="Offset X"
            value={settings.offsetX}
            onChange={onXOffsetChange}
            min={-19}
            max={19}
            step={1}
          />
        </div>
        <div className="flex items-center">
          <NumberInput
            label="Offset Y"
            value={settings.offsetY}
            onChange={onYOffsetChange}
            min={-19}
            max={19}
            step={1}
          />
        </div>
        <div className="flex items-center pb-5">
          <Expandable
            expandedClassName="p-0"
            arrow={false}
            initialOpen={colorpickerOpen}
            collapsedContent={
              <div className="flex justify-between">
                <div className="">Color</div>
                <div>{colorpickerPreview}</div>
              </div>
            }
            expandedContent={
              <HexColorPicker
                className="p-5"
                color={settings.color}
                onChange={(color) => onColorChange(color)}
              />
            }
          />
        </div>
        <div className="flex justify-end mb-5 -mt-[30px]">
          <button
            className="btn btn-sm  btn-outline btn-error cursor-pointer"
            onClick={() => removeTextItem(settings.line)}
          >
            Delete
          </button>
        </div>
      </div>
    );
  }
);
