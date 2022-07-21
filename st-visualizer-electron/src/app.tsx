import * as React from "react";

import { App } from "../imports/App";
import { createRoot } from "react-dom/client";

function render() {
  const target = document.getElementById("react-target");
  const root = target && createRoot(target);
  root?.render(<App />);
}

render();
