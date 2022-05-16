import React, { useEffect, useState } from "react";
import { config } from "../../../config";
import { Loader } from "./Loader";

import "./VersionChecker.less";

const localVersion = (window as any).version;
interface Props {}

const hasAuthCredentials = () => config.updates.credentials && config.updates.credentials.username && config.updates.credentials.password;

const getDownloadUrl = () => {
	if (hasAuthCredentials()) {
		const [protocol, domain] = config.updates.downloadUrl.split("://");
		return `${protocol}://${config.updates.credentials.username}:${config.updates.credentials.password}@${domain}`;
	} else {
		return config.updates.downloadUrl;
	}
};

export const VersionChecker: React.FC<Props> = () => {
	const [didUpdateCheck, setUpdateCheck] = useState(false);
	const [versionNumber, setNewVersionNumber] = useState(localVersion);
	const [newVersionAvailable, setNewVersionAvailable] = useState(false);

	useEffect(() => {
		const fetchConfig: any = { method: "GET" };

		if (hasAuthCredentials()) {
			let headers = new Headers();
			headers.set("Authorization", "Basic " + window.btoa(config.updates.credentials.username + ":" + config.updates.credentials.password));
			fetchConfig.headers = headers;
		}

		fetch(config.updates.versionUrl, fetchConfig)
			.then((response) => response.json())
			.then((json) => {
				setUpdateCheck(true);

				if (json[0].version !== localVersion) {
					setNewVersionNumber(json[0].version);
					setNewVersionAvailable(true);
				}
			});
	}, []);

	return (
		<div className="version-checker">
			{newVersionAvailable && (
				<a className="new-version" href={getDownloadUrl()} target="_blank">{`New Version ${versionNumber} available!`}</a>
			)}
			{!newVersionAvailable && <div>{`version ${versionNumber}`}</div>}
			{!didUpdateCheck && <Loader />}
		</div>
	);
};
