import React, { useEffect, useState } from "react";
import { Loader } from "../../utils/Loader";

import "./RandomImageList.css";
import {
  getApiKeyFromLocalStorage,
  saveApiKeyToLocalStorage,
} from "../../../utils/storage";
import { config } from "../../../../config";

import { RefreshCw } from "lucide-react";

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

  const initialApiKey: string | null =
    config.unsplashApiKey === ""
      ? getApiKeyFromLocalStorage()
      : config.unsplashApiKey;
  const [apiKey, setApiKey] = useState(initialApiKey);
  const [newApiKey, setNewApiKey] = useState("");

  const [apiError, setApiError] = useState(false);

  useEffect(() => {
    if (apiKey) {
      console.log("load images");
      loadImages();
    }
  }, [reloadId, apiKey]);

  const loadImages = async () => {
    setIsLoading(true);

    try {
      const response = await fetch(
        `https://api.unsplash.com/photos/random?client_id=${apiKey}&orientation=landscape&count=10`
      );
      const data = await response.json();

      img1 = data[0];
      const images: UnsplashImage[] = data.map((d: any) => ({
        downloadUrl: d.links.download_location,
        thumbUrl: d.urls.thumb,
        userName: d.user.first_name + " " + d.user.last_name,
        userUrl: d.user.links.html,
      }));

      setImages(images);
      setIsLoading(false);
    } catch (e) {
      setApiError(true);
    }
  };

  const getNewImages = () => setReloadId(reloadId + 1);

  const onImageSelected = (image: UnsplashImage) => {
    setSelectedImage(image);
    imageSelected(image.thumbUrl);

    // Track unsplash img download as required in their ToS
    fetch(`${image.downloadUrl}?client_id=${config.unsplashApiKey}`)
      .then((res) => res.json())
      .then(console.log);
  };

  const onApiKeySave = () => {
    if (newApiKey.trim().length > 0) {
      saveApiKeyToLocalStorage(newApiKey);
      setApiKey(newApiKey);
      setApiError(false);
    }
  };

  const renderImageList = () => {
    return (
      <>
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
      </>
    );
  };

  const renderApiInput = () => {
    return (
      <div className="api-key-input">
        {apiError && (
          <div className="api-error-msg">
            The unsplash API returned an error.
          </div>
        )}
        <div>
          Please enter your{" "}
          <a href="https://unsplash.com/developers" target="_blank">
            unsplash API key
          </a>
        </div>
        <input
          type="text"
          onChange={(e) => setNewApiKey((e.target as HTMLInputElement).value)}
        />
        <button onClick={onApiKeySave}>Set api key</button>
      </div>
    );
  };

  return (
    <div className="random-images">
      {apiKey && !apiError && (
        <div className="title">
          <span>Random Images</span>
          <RefreshCw
            size={16}
            onClick={getNewImages}
            className="icon-clickable"
          />
        </div>
      )}
      {apiKey && !apiError ? renderImageList() : renderApiInput()}
      {selectedImage && (
        <div className="selected-image-credits">
          <span>Photo by</span>
          <a
            href={`${selectedImage.userUrl}?utm_source=Led_Clock&utm_medium=referral`}
          >
            {selectedImage.userName}
          </a>
          <span>on</span>
          <a href="https://unsplash.com/?utm_source=yLed_Clock&utm_medium=referral">
            Unsplash
          </a>
        </div>
      )}
    </div>
  );
};
