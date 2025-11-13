import React, {
  forwardRef,
  JSX,
  PropsWithChildren,
  useEffect,
  useImperativeHandle,
  useRef,
  useState,
} from "react";
import { ChevronDown } from "lucide-react";

interface Props extends PropsWithChildren {
  initialOpen?: boolean;
  title: string;
  titleElement?: JSX.Element;
  className?: string;
  onExpand?: (isExpanded: boolean) => void;
  ref?: any;
}

export const Expandable: React.FC<Props> = forwardRef(
  (
    { initialOpen, title, children, titleElement, className, onExpand }: Props,
    ref
  ) => {
    const [active, setActive] = useState(initialOpen);
    const [height, setHeight] = useState<number>();
    const content = useRef<HTMLDivElement>();

    useEffect(() => {
      if (content && content.current) {
        setHeight(content.current.scrollHeight);
      }
    }, [active, height]);

    useImperativeHandle(ref, () => ({
      updateHeight: () => {
        setHeight(content.current.scrollHeight);
      },
    }));

    const toggleActive = () => {
      const isActive = !active;
      setActive(isActive);

      if (onExpand) {
        onExpand(isActive);
      }
    };

    return (
      <div className={`flex flex-grow flex-col ${className || ""}`}>
        <div
          className="cursor-pointer flex items-center border-none outline-none mb-5 transition-[background-color] duration-300"
          onClick={toggleActive}
        >
          <p className="m-0 flex-grow">{title}</p>
          {titleElement}
          <ChevronDown
            className={`ml-auto transition-transform duration-[600ms] ${
              active ? "rotate-0" : "-rotate-90"
            }`}
          />
        </div>
        <div
          ref={content}
          style={{ maxHeight: `${active ? height : 0}px` }}
          className="overflow-hidden transition-[max-height] duration-[600ms]"
        >
          <div>{children}</div>
        </div>
      </div>
    );
  }
);
