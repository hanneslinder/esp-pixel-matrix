import React, { DragEvent, FormEvent, JSX } from "react";
import { view } from "@risingstack/react-easy-state";
import { useState } from "react";

import { ImagePlus } from "lucide-react";

interface FilePickerProps {
  onFileDroppedOrSelected: (file: File) => void;
  /**
   * Receives the whole File, not just its MIME type: browsers derive `type`
   * from the OS association, so a .bin can arrive as application/octet-stream,
   * application/macbinary, text/plain or "" depending on the machine.
   */
  isFileTypeAllowed: (file: File) => boolean;
  label: string;
  progress?: number;
  icon?: JSX.Element;
}

export const FilePicker = view(
  ({
    onFileDroppedOrSelected,
    isFileTypeAllowed,
    label,
    progress,
    icon,
  }: FilePickerProps) => {
    const uploadRef = React.useRef<HTMLInputElement>(null);
    const [rejected, setRejected] = useState<string | null>(null);

    // A silently ignored file looks like a broken button, so always say why.
    const accept = (file: File | undefined) => {
      if (!file) {
        return;
      }

      if (isFileTypeAllowed(file)) {
        setRejected(null);
        onFileDroppedOrSelected(file);
      } else {
        console.warn("Rejected file", file.name, "with type", file.type);
        setRejected(file.name);
      }
    };

    const onImageSelected = (evt: FormEvent<HTMLDivElement>) => {
      const target = evt.target as HTMLInputElement;
      accept(target?.files?.[0]);
      // Allow re-selecting the same file after a rejection.
      if (target) {
        target.value = "";
      }
    };

    const handleDragEnter = (e: DragEvent<HTMLDivElement>) => {
      e.preventDefault();
    };

    const handleDragLeave = (e: DragEvent<HTMLDivElement>) => {
      e.preventDefault();
    };

    const handleDragOver = (e: DragEvent<HTMLDivElement>) => {
      e.preventDefault();
    };

    const handleDrop = (e: DragEvent<HTMLDivElement>) => {
      e.preventDefault();
      accept(e.dataTransfer.files?.[0]);
    };

    const renderInputHint = () => (
      <>
        <div className="p-5">
          {icon ? icon : <ImagePlus title="Select image" />}
        </div>
        <div className="text-xs">{label}</div>
        <div className="text-xs">or</div>
        <div>
          <label
            htmlFor="upload-input"
            className="cursor-pointer p-2 mt-2 block"
          >
            <button
              className="btn btn-xs btn-soft"
              onClick={() => uploadRef.current?.click()}
            >
              Choose a file
            </button>
          </label>
          <input
            id="upload-input"
            className="hidden"
            type="file"
            ref={uploadRef}
            onChange={onImageSelected}
          />
        </div>
        {rejected && (
          <div className="text-xs text-error mt-2 text-center px-2">
            {`"${rejected}" is not a supported file`}
          </div>
        )}
      </>
    );

    const renderProgress = () => (
      <div className="text-center">
        {progress < 100 ? (
          <>
            <div>Updating Firmware</div>
            <div>Don't leave page until finished!</div>
            <progress max="100" value={progress} className="mt-5" />
            <div>{`${progress}%`}</div>
          </>
        ) : (
          <div>
            <div>Firmware update done!</div>
            <div>Page will reload in 5s.</div>
          </div>
        )}
      </div>
    );

    return (
      <div
        className={`text-xs flex-grow-0 flex justify-center items-center flex-col h-[200px] border-2 border-dashed rounded-md ${
          progress ? "pointer-events-none" : ""
        }`}
        onDrop={(e) => handleDrop(e)}
        onDragOver={(e) => handleDragOver(e)}
        onDragEnter={(e) => handleDragEnter(e)}
        onDragLeave={(e) => handleDragLeave(e)}
      >
        {!progress ? renderInputHint() : renderProgress()}
      </div>
    );
  }
);
