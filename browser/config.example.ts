export const config = {
	unsplashApiKey: "xxx-xxx-xxx-xxx", // Add unsplash api key if you want to be able to query random images https://unsplash.com/developers
	updates: {
		versionUrl: "", // check for new version updates from this url, e.g. https://example.com/pixelmatrix/version.json
		downloadUrl: "", // page listing the firmware downloads, e.g. https://example.com/pixelmatrix/downloads.html
		// (optional) bearer token, sent as `Authorization: Bearer <token>`.
		// Leave empty when the update endpoints are publicly reachable.
		bearerToken: "",
		// (optional) username for the Basic fallback, used when the download
		// page is opened in a tab: browsers cannot send a bearer header on a
		// navigation, but they will use credentials embedded in the URL. The
		// server accepts Basic <any username>:<bearerToken>.
		basicUsername: "pixelmatrix",
	},
};
