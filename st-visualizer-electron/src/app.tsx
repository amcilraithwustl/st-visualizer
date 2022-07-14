import * as React from "react";

import { App } from "../imports/App";
import { createRoot } from "react-dom/client";

function render() {
  const root = createRoot(document.getElementById("react-target"));
  root.render(<App />);
}

render();
