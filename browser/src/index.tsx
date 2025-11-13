import React, { useRef } from "react";
import ReactDOM from "react-dom";
import { view } from "@risingstack/react-easy-state";

import { CanvasWrapper, CanvasWrapperElement } from "./components/canvas/CanvasWrapper";
import { Header } from "./components/Header";
import { SidebarRight } from "./components/sidebar-right/SidebarRight";
import { SidebarLeft } from "./components/sidebar-left/SidebarLeft";

import "./index.css";

const App = view(() => {
	const canvasWrapperRef = useRef<CanvasWrapperElement>(null);

	return (
		<div className="layout">
			<Header />
			<div className="content">
				<div className="sidebar-left">
					<SidebarLeft />
				</div>
				<div className="main-content">
					<CanvasWrapper ref={canvasWrapperRef} />
				</div>
				<div className="sidebar-right">
					<SidebarRight getCanvas={() => canvasWrapperRef.current.canvas} />
				</div>
				<div className="toolbar-top"></div>
			</div>
		</div>
	);
});

ReactDOM.render(<App />, document.getElementById("app"));
