import React, { useEffect, useState } from "react";
import { Loader } from "../../utils/Loader";
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
      <div className="overflow-scroll flex-shrink flex-grow basis-[200px]">
        {isLoading && (
          <div className="w-10 h-10 border-2">
            <Loader />
          </div>
        )}
        <div className="max-h-[550px] grid grid-cols-2 gap-2.5">
          {!isLoading &&
            images.map((i: UnsplashImage) => (
              <div
                key={i.downloadUrl}
                className="w-full h-[100px] bg-cover cursor-pointer"
                style={{ backgroundImage: `url(${i.thumbUrl})` }}
                onClick={() => onImageSelected(i)}
              />
            ))}
        </div>
      </div>
    );
  };

  const renderApiInput = () => {
    return (
      <div>
        {apiError && (
          <div className="text-xs text-[#e06262] mb-2.5">
            The unsplash API returned an error.
          </div>
        )}
        <div>
          Please enter your{" "}
          <a
            href="https://unsplash.com/developers"
            target="_blank"
            className="text-[--color-highlight-2]"
          >
            unsplash API key
          </a>
        </div>
        <input
          type="text"
          className="mt-2.5 w-full bg-[--color-dark-1] text-[--color-text] px-2.5 py-1.5 border-none h-[30px] rounded-md"
          onChange={(e) => setNewApiKey((e.target as HTMLInputElement).value)}
        />
        <button
          className="cursor-pointer bg-[--color-dark-1] text-[--color-text] px-2.5 py-2.5 mt-2.5 block border-none rounded-md"
          onClick={onApiKeySave}
        >
          Set api key
        </button>
      </div>
    );
  };

  return (
    <div>
      {apiKey && !apiError && (
        <div className="flex justify-center items-center mb-2.5 text-center text-xs [&>span]:p-2.5 [&>svg]:w-5 [&>svg]:p-2.5 [&>svg]:inline-block [&>svg]:translate-y-[-2px]">
          <span>Random Images</span>
          <RefreshCw
            size={16}
            onClick={getNewImages}
            className="cursor-pointer [&>svg]:stroke-[1px]"
          />
        </div>
      )}
      {apiKey && !apiError ? renderImageList() : renderApiInput()}
      {selectedImage && (
        <div className="hidden text-[--color-text-2] text-xs mb-2.5 [&>a]:text-[--color-text-2] [&>a]:px-1.5">
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
