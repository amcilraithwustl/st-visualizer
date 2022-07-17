import "react";
import * as React from "react";
import { CustomRenderer } from "./ui/threejsComponents/CustomRenderer";


export const App = () => {
  return (
    <div>
      React/Webgl Example
      <div
        style={{ borderWidth: 3, borderColor: "black", borderStyle: "solid" }}
      >
        <CustomRenderer />
      </div>
    </div>
  );
};
