import "react";
import * as React from "react";
import { CustomRenderer } from "./ui/CustomRenderer";

export const App = () => {
  return (
    <div
      style={{
        padding: 10,
        boxSizing: "border-box",
        width: "100%",
        height: "100%",
      }}
    >
      <div style={{ width: "100%" }}>
        <CustomRenderer />
      </div>
    </div>
  );
};
