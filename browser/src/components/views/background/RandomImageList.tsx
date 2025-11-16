import React, { useEffect, useState } from "react";
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
      <div className="flex-grow overflow-auto">
        {isLoading && (
          <div className="flex items-center justify-center">
            <span className="loading loading-spinner loading-md" />
          </div>
        )}
        <div className="max-h-[550px] grid grid-cols-2 gap-2.5">
          {!isLoading &&
            images.map((i: UnsplashImage) => (
              <div
                key={i.downloadUrl}
                className="w-full h-[100px] bg-cover cursor-pointer relative mx-auto overflow-hidden rounded-md border-1 border-gray-700 hover:border-gray-100"
                onClick={() => onImageSelected(i)}
              >
                <img
                  src={i.thumbUrl}
                  className="w-full h-full object-cover hover:opacity-80w-full h-auto relative z-0 rounded-lg transition-all duration-300 hover:scale-110"
                />
              </div>
            ))}
        </div>
      </div>
    );
  };

  const renderApiInput = () => {
    return (
      <div>
        {apiError && (
          <div className="text-xs text-[#e06262] mb-2">
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
          className="btn btn-sm"
          onChange={(e) => setNewApiKey((e.target as HTMLInputElement).value)}
        />
        <button className="btn btn-sm" onClick={onApiKeySave}>
          Set api key
        </button>
      </div>
    );
  };

  return (
    <div className="flex flex-col overflow-auto">
      {apiKey && !apiError && (
        <div className="flex justify-center items-center mb-2 text-center text-xs">
          <span>Random Images</span>
          <RefreshCw
            size={16}
            onClick={getNewImages}
            className="cursor-pointer ml-2"
          />
        </div>
      )}
      {apiKey && !apiError ? renderImageList() : renderApiInput()}
      {selectedImage && (
        <div className="hidden">
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
