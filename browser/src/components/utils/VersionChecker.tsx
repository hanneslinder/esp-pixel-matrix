import React, { useEffect, useState } from "react";
import { config } from "../../../config";
import { Loader } from "./Loader";

import "./VersionChecker.less";

const localVersion = (window as any).version;
interface Props {}

export const VersionChecker: React.FC<Props> = () => {
	const [didUpdateCheck, setUpdateCheck] = useState(false);
	const [versionNumber, setNewVersionNumber] = useState(localVersion);
	const [newVersionAvailable, setNewVersionAvailable] = useState(false);

	useEffect(() => {
		fetch(config.updates.versionUrl)
			.then((response) => response.json())
			.then((json) => {
				console.log(json);
				setUpdateCheck(true);

				if (json[0].version !== localVersion) {
					setNewVersionNumber(json[0].version);
					setNewVersionAvailable(true);
				}
			});
	}, [didUpdateCheck]);

	return (
		<div className="version-checker">
			{newVersionAvailable && <a className="new-version" href={config.updates.downloadUrl} target="_blank">{`New Version ${versionNumber} available!`}</a>}
			{!newVersionAvailable && <div>{`version ${versionNumber}`}</div>}
			{!didUpdateCheck && <Loader />}
		</div>
	);
};
