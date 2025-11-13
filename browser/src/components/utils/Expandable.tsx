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

import "./Expandable.css";

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
      <div className={`expandable-section ${className}`}>
        <div
          className={`expandable ${active ? "is-expanded" : "is-collapsed"}`}
          onClick={toggleActive}
        >
          <p className="expandable-title">{title}</p>
          {titleElement}
          <ChevronDown
            className={`expandable-icon ${active ? "rotate" : ""}`}
          />
        </div>
        <div
          ref={content}
          style={{ maxHeight: `${active ? height : 0}px` }}
          className="expandable-content"
        >
          <div className="expandable-children">{children}</div>
        </div>
      </div>
    );
  }
);
