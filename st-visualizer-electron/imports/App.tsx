import "react";
import * as React from "react";
import { ItemToRender } from "./ui/threejsComponents/ItemToRender";


export const App = () => {
  return (
    <div>
      React/Webgl Example
      <div
        style={{ borderWidth: 3, borderColor: "black", borderStyle: "solid" }}
      >
        <ItemToRender />
      </div>
    </div>
  );
};
