import React, { DragEvent, FormEvent } from "react";
import { view } from "@risingstack/react-easy-state";

import { ImagePlus } from "lucide-react";

interface Props {
  onFileDroppedOrSelected: (file: File) => void;
  isFileTypeAllowed: (mimeType: string) => boolean;
  label: string;
  progress?: number;
}

export const FilePicker: React.FC<Props> = view(
  ({ onFileDroppedOrSelected, isFileTypeAllowed, label, progress }) => {
    const onImageSelected = (evt: FormEvent<HTMLDivElement>) => {
      const target = (evt.target as HTMLInputElement)!;

      if (
        target &&
        target.files &&
        target.files.length > 0 &&
        isFileTypeAllowed(target.files[0].type)
      ) {
        onFileDroppedOrSelected(target.files[0]);
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

      if (isFileTypeAllowed(e.dataTransfer.files[0].type)) {
        onFileDroppedOrSelected(e.dataTransfer.files[0]);
      }
    };

    const renderInputHint = () => (
      <>
        <div className="p-5">
          <ImagePlus title="Select image" />
        </div>
        <div className="text-xs">{label}</div>
        <div className="text-xs">or</div>
        <div>
          <label
            htmlFor="upload-input"
            className="cursor-pointer p-2 mt-2 block"
          >
            <span>Choose a file</span>
          </label>
          <input
            id="upload-input"
            className="hidden"
            type="file"
            onChange={onImageSelected}
          />
        </div>
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
