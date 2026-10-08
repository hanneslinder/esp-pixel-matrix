import { view } from "@risingstack/react-easy-state";
import React from "react";
import { useEffect, useState } from "react";
import { getSocket, IncommingMessageType } from "../../../Websocket";
import { FilePicker } from "../../utils/FilePicker";
import { FileCog } from "lucide-react";

interface ProgressUpdateData {
  type: IncommingMessageType.UpdateProgress;
  progress: number;
}

/**
 * Both OTA payloads are .bin files, and the browser's MIME type for those is
 * whatever the OS decides (application/octet-stream, application/macbinary,
 * text/plain, sometimes ""), so trusting `type` rejects perfectly good files.
 * The device routes by filename anyway - see isFileSystemImage() in
 * OTAUpdateHandler.cpp - so match on the name.
 */
const isFirmwareFile = (file: File) => file.name.toLowerCase().endsWith(".bin");
const matrixIP = (window as any).websocketUrl;

async function sendData(data: File) {
  const formData = new FormData();
  formData.append("update", data);

  const response = await fetch(`http://${matrixIP}/doUpdate`, {
    method: "POST",
    body: formData,
  });

  return response;
}

interface Props {}

export const FirmwareUpdateForm: React.FC<Props> = view(() => {
  let unsubscribeFromProgress: () => void;

  const [updateProgress, setProgress] = useState(undefined);

  const handleFile = (file: File) => {
    sendData(file);
  };

  const onUpdateProgress = (data: ProgressUpdateData) => {
    console.log("UpdateProgress", data.progress);

    if (data.progress) {
      if (data.progress >= 95) {
        setProgress(100);
        setTimeout(() => window.location.reload(), 5000);
      } else {
        setProgress(data.progress);
      }
    }
  };

  useEffect(() => {
    const socket = getSocket();
    unsubscribeFromProgress = socket.subscribe(
      IncommingMessageType.UpdateProgress,
      onUpdateProgress
    );

    return function cleanup() {
      unsubscribeFromProgress();
    };
  }, [unsubscribeFromProgress]);

  return (
    <div className="firmware-update-form">
      <FilePicker
        onFileDroppedOrSelected={handleFile}
        isFileTypeAllowed={isFirmwareFile}
        label="Drag and drop firmware here"
        progress={updateProgress}
        icon={<FileCog title="Firmware Update" />}
      />
    </div>
  );
});
