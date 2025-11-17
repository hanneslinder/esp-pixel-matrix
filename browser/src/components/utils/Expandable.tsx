import clsx from "clsx";
import { ChevronDown, ChevronRight } from "lucide-react";
import { JSX } from "preact/jsx-runtime";

interface ExpandableProps {
  collapsedContent: JSX.Element;
  expandedContent: JSX.Element;
  initialOpen?: boolean;
  arrow?: boolean;
  containerClassName?: string;
  collapsedClassName?: string;
  expandedClassName?: string;
}

export const Expandable = ({
  collapsedContent,
  expandedContent,
  arrow = true,
  initialOpen = false,
  containerClassName,
  collapsedClassName,
  expandedClassName,
}: ExpandableProps) => {
  return (
    <div
      className={clsx(
        "collapse",
        arrow && "collapse-arrow",
        containerClassName
      )}
    >
      <input type="checkbox" className="peer" checked={initialOpen} />
      <div
        className={clsx(
          "collapse-title !p-0 text-primary-content",
          collapsedClassName
        )}
      >
        {collapsedContent}
      </div>
      <div
        className={clsx(
          "collapse-content text-primary-content",
          expandedClassName
        )}
      >
        {expandedContent}
      </div>
    </div>
  );
};
