import React, { DragEvent, FormEvent } from "react";
import { view } from "@risingstack/react-easy-state";
import { SvgIcon } from "./SvgIcon";

import "./FilePicker.less";

const iconAddFile = require("../../assets/cloud-plus.svg");

interface Props {
	onFileDroppedOrSelected: (file: File) => void;
	isFileTypeAllowed: (mimeType: string) => boolean;
	label: string;
	progress?: number;
}

export const FilePicker: React.FC<Props> = view(({ onFileDroppedOrSelected, isFileTypeAllowed, label, progress }) => {
	const onImageSelected = (evt: FormEvent<HTMLDivElement>) => {
		const target = (evt.target as HTMLInputElement)!;

		if (target && target.files && target.files.length > 0 && isFileTypeAllowed(target.files[0].type)) {
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
			<SvgIcon icon={iconAddFile} title="Select image" />
			<div className="sub-header">{label}</div>
			<div>or</div>
			<div className="file-picker">
				<label htmlFor="upload-input" className="file-picker-label">
					<span className="background-file-button">Chose a file</span>
				</label>
				<input id="upload-input" className="file-picker-input" type="file" onChange={onImageSelected} />
			</div>
		</>
	);

	const renderProgress = () => (
		<div className="update-progress">
			{progress < 100 ? (
				<>
					<div>Updating Firmware</div>
					<div>Don't leave page util finished!</div>
					<progress max="100" value={progress} />
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
			className={`background-file-picker ${progress ? "is-uploading" : ""}`}
			onDrop={(e) => handleDrop(e)}
			onDragOver={(e) => handleDragOver(e)}
			onDragEnter={(e) => handleDragEnter(e)}
			onDragLeave={(e) => handleDragLeave(e)}
		>
			{!progress ? renderInputHint() : renderProgress()}
		</div>
	);
});
