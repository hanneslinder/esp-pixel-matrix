import React, { useEffect, useState } from "react";
import { config } from "../../../../config";
import { Loader } from "../../utils/Loader";
import { SvgIcon } from "../../utils/SvgIcon";

import "./RandomImageList.less";

const iconReload = require("../../../assets/arrow-clockwise.svg");

interface Props {
	imageSelected: (url: string) => void;
}

interface UnsplashImage {
	downloadUrl: string;
	thumbUrl: string;
	userName: string;
	userUrl: string;
}

let img1;
export const RandomImageList: React.FC<Props> = ({ imageSelected }) => {
	const [images, setImages] = useState([]);
	const [isLoading, setIsLoading] = useState(true);
	const [reloadId, setReloadId] = useState(0);
	const [selectedImage, setSelectedImage]: [UnsplashImage, any] = useState();

	useEffect(() => {
		setIsLoading(true);

		fetch(`https://api.unsplash.com/photos/random?client_id=${config.unsplashApiKey}&orientation=landscape&count=10`)
			.then((response) => {
				return response.json();
			})
			.then((data) => {
				console.log(data);
				img1 = data[0];
				const images: UnsplashImage[] = data.map((d: any) => ({
					downloadUrl: d.links.download_location,
					thumbUrl: d.urls.thumb,
					userName: d.user.first_name + " " + d.user.last_name,
					userUrl: d.user.links.html,
				}));
				setImages(images);
				setIsLoading(false);
			});
	}, [reloadId]);

	const getNewImages = () => setReloadId(reloadId + 1);

	const onImageSelected = (image: UnsplashImage) => {
		setSelectedImage(image);
		imageSelected(image.thumbUrl);

		// Track unsplash img download as required in their ToS
		fetch(`${image.downloadUrl}?client_id=${config.unsplashApiKey}`)
			.then((res) => res.json())
			.then(console.log);
	};

	return (
		<div className="random-images">
			<div className="title">
				<span>Random Images</span>
				<SvgIcon className="sidebar-icon" icon={iconReload} onClick={getNewImages} />
			</div>
			{isLoading && <Loader />}
			<div className="random-image-list">
				{!isLoading &&
					images.map((i: UnsplashImage) => (
						<div
							key={i.downloadUrl}
							className="random-image"
							style={{ backgroundImage: `url(${i.thumbUrl})` }}
							onClick={() => onImageSelected(i)}
						/>
					))}
			</div>
			{selectedImage && (
				<div className="selected-image-credits">
					<span>Photo by</span>
					<a href={`${selectedImage.userUrl}?utm_source=Led_Clock&utm_medium=referral`}>{selectedImage.userName}</a>
					<span>on</span>
					<a href="https://unsplash.com/?utm_source=yLed_Clock&utm_medium=referral">Unsplash</a>
				</div>
			)}
		</div>
	);
};
