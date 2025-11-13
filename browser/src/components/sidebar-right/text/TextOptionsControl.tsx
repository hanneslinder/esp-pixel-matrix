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
        className="color-picker-preview"
        style={{ background: settings.color }}
      />
    );

    console.log(settings);

    return (
      <div className="sidebar-settings-section">
        <div className="sidebar-settings-item">
          <div className="sidebar-settings-label">
            <span>Text</span>
            <MessageCircleQuestionMark
              className="icon-text-help"
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
          <div className="sidebar-settings-input">
            <input
              type="text"
              value={settings.text}
              onChange={(e) =>
                onTextChange((e.target as HTMLInputElement).value)
              }
            />
          </div>
        </div>
        <div className="sidebar-settings-item">
          <div className="sidebar-settings-label">Font</div>
          <div className="sidebar-settings-input">
            <Dropdown
              selected={settings.font}
              items={getFontItems()}
              onSelect={onFontChange}
            />
          </div>
        </div>
        <div className="sidebar-settings-item">
          <div className="sidebar-settings-label">Size</div>
          <div className="sidebar-settings-input">
            <div className="offset-input">{settings.size}</div>
            <div className="offset-buttons">
              <div
                className={`offset-icon decrement ${
                  settings.size === 1 ? "is-disabled" : ""
                }`}
                onClick={() => changeSize(settings.size - 1)}
              >
                -
              </div>
              <div
                className={`offset-icon increment ${
                  settings.size > 4 ? "is-disabled" : ""
                }`}
                onClick={() => changeSize(settings.size + 1)}
              >
                +
              </div>
            </div>
          </div>
        </div>
        <div className="sidebar-settings-item">
          <div className="sidebar-settings-label">Align</div>
          <div className="sidebar-settings-input">
            <Dropdown
              selected={settings.align}
              items={getTextAlignItems()}
              onSelect={onAlignChange}
            />
          </div>
        </div>
        <div className="sidebar-settings-item">
          <div className="sidebar-settings-label">Offset X</div>
          <div className="sidebar-settings-input">
            <div className="offset-input">{settings.offsetX}</div>
            <div className="offset-buttons">
              <div
                className={`offset-icon decrement ${
                  settings.offsetX < -19 ? "is-disabled" : ""
                }`}
                onClick={() => onXOffsetChange(settings.offsetX - 1)}
              >
                -
              </div>
              <div
                className={`offset-icon increment ${
                  settings.offsetX > 19 ? "is-disabled" : ""
                }`}
                onClick={() => onXOffsetChange(settings.offsetX + 1)}
              >
                +
              </div>
            </div>
          </div>
        </div>
        <div className="sidebar-settings-item">
          <div className="sidebar-settings-label">Offset Y</div>
          <div className="sidebar-settings-input">
            <div className="settings-input">{settings.offsetY}</div>
            <div className="offset-buttons">
              <div
                className={`offset-icon decrement ${
                  settings.offsetY < -19 ? "is-disabled" : ""
                }`}
                onClick={() => onYOffsetChange(settings.offsetY - 1)}
              >
                -
              </div>
              <div
                className={`offset-icon increment ${
                  settings.offsetY > 19 ? "is-disabled" : ""
                }`}
                onClick={() => onYOffsetChange(settings.offsetY + 1)}
              >
                +
              </div>
            </div>
          </div>
        </div>
        <div className="sidebar-settings-item settings-color">
          <Expandable
            title="Color"
            initialOpen={false}
            titleElement={clorpickerPreview}
            onExpand={updateHeightOnAnimationEnd}
          >
            <HexColorPicker
              color={settings.color}
              onChange={(color) => onColorChange(color)}
            />
          </Expandable>
        </div>
        <div className="btn-delete-text">
          <button onClick={() => removeTextItem(settings.line)}>delete</button>
        </div>
      </div>
    );
  }
);
