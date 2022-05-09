import { RefObject, useEffect, useRef, useState } from "react";

const useFade = (initial: boolean) => {
	const [show, setShow] = useState(initial);
	const [isVisible, setVisible] = useState(show);

	useEffect(() => {
		if (show) setVisible(true);
	}, [show]);

	const onAnimationEnd = () => {
		if (!show) setVisible(false);
	};

	const style = { animation: `${show ? "fadeIn" : "fadeOut"} .3s` };

	const fadeProps = {
		style,
		onAnimationEnd,
	};

	return [isVisible, setShow, fadeProps];
};

// From https://usehooks-ts.com/react-hook/use-event-listener
export function useEventListener<T extends HTMLElement = HTMLDivElement>(
	eventName: keyof WindowEventMap | string,
	handler: (event: Event) => void,
	element?: RefObject<T>
) {
	const savedHandler = useRef<(event: Event) => void>();

	useEffect(() => {
		const targetElement: T | Window = element?.current || window;
		if (!(targetElement && targetElement.addEventListener)) {
			return;
		}

		if (savedHandler.current !== handler) {
			savedHandler.current = handler;
		}

		const eventListener = (event: Event) => {
			// eslint-disable-next-line no-extra-boolean-cast
			if (!!savedHandler?.current) {
				savedHandler.current(event);
			}
		};

		targetElement.addEventListener(eventName, eventListener);

		// Remove event listener on cleanup
		return () => {
			targetElement.removeEventListener(eventName, eventListener);
		};
	}, [eventName, element, handler]);
}

type Handler = (event: MouseEvent) => void;

// From https://usehooks-ts.com/react-hook/use-on-click-outside
export function useOnClickOutside<T extends HTMLElement = HTMLElement>(
	ref: RefObject<T>,
	handler: Handler,
	mouseEvent: "mousedown" | "mouseup" = "mousedown"
): void {
	useEventListener(mouseEvent, (event) => {
		const el = ref?.current;

		if (!el || el.contains(event.target as Node)) {
			return;
		}

		handler(event as unknown as MouseEvent);
	});
}
