import { view } from "@risingstack/react-easy-state";
import { HexColorPicker } from "react-colorful";
import { setTextAction } from "../../../Actions";
import { limitNumberBetween } from "../../../utils/utils";
import { Expandable } from "../../utils/Expandable";

import { MessageCircleQuestionMark } from "lucide-react";
import { DropdownItem, Dropdown } from "../../utils/Dropdown";
import {
  appState,
  Font,
  TextAlign,
  TextOptions,
} from "../../../state/appState";
import { NumberInput } from "./NumberInput";

const getTextAlignItems = (): { [key: string]: TextAlign } => ({
  Left: TextAlign.LEFT,
  Center: TextAlign.CENTER,
  Right: TextAlign.RIGHT,
});

const getFontMap = (): { [key: string]: Font } => ({
  Regular: Font.REGULAR,
  Pico: Font.PICO,
});

let colorChangeTimeout: ReturnType<typeof setTimeout>;
const COLORPICKER_TIMEOUT_MS = 350;

interface TextOptionsControlProps {
  settings: TextOptions;
  onResize?: () => void;
}

export const TextOptionsControl = view(
  ({ settings, onResize }: TextOptionsControlProps) => {
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

    const onSizeChange = (size: number) => {
      const newSettings = { ...settings, size };
      updateTextItem(newSettings);
    };

    const onFontChange = (fontName: string) => {
      const font = getFontMap()[fontName];
      const newSettings = { ...settings, font };
      updateTextItem(newSettings);
    };

    const onAlignChange = (alignName: string) => {
      const align = getTextAlignItems()[alignName];
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
              defaultValue="Font"
              className="select bg-gray-900 hover:bg-gray-700"
              onChange={(e) => onFontChange(e.currentTarget.value)}
            >
              <option>Regular</option>
              <option>Pico</option>
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
              defaultValue="Text Align"
              className="select bg-gray-900"
              onChange={(e) => onAlignChange(e.currentTarget.value)}
            >
              <option>Left</option>
              <option>Center</option>
              <option>Right</option>
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
            title="Color"
            initialOpen={false}
            titleElement={clorpickerPreview}
            onExpand={updateHeightOnAnimationEnd}
          >
            <HexColorPicker
              className="p-5"
              color={settings.color}
              onChange={(color) => onColorChange(color)}
            />
          </Expandable>
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
