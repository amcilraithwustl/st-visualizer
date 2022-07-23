import "react";
import * as React from "react";
import { CustomRenderer } from "./ui/CustomRenderer";

export const App = () => {
  return (
    <div>
      React/Webgl Example
      <div style={{ width: "100%" }}>
        <CustomRenderer />
      </div>
    </div>
  );
};
