import { Tabs, Tab } from "@mui/material";
import "react";
import * as React from "react";
import { useState } from "react";
import { datatype, importPts } from "./api/constants";
import { CustomRenderer } from "./ui/CustomRenderer";
import { ImportPage } from "./ui/ImportPage/ImportPage";
const Hidden = ({
  on,
  children,
}: {
  on: boolean;
  children: JSX.Element | null;
}) => {
  return <div style={{ display: on ? undefined : "none" }}>{children}</div>;
};

export const App = () => {
  const [value, setValue] = useState(0);
  const handleChange = (event: React.SyntheticEvent, newValue: number) => {
    setValue(newValue);
  };
  const [data, setData] = useState<datatype | undefined>(undefined);
  React.useEffect(() => {
    importPts().then((res) => setData(res));
  }, []);
  console.log(window.electronAPI);
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
        <Tabs value={value} onChange={handleChange}>
          <Tab label="Data Import" id={0 + "tab"} />
          <Tab label="Data Alignment" id={1 + "tab"} />
          <Tab label="Data Display" id={2 + "tab"} />
        </Tabs>
        <Hidden on={value === 0}>
          <ImportPage />
        </Hidden>
        <Hidden on={value == 1}>
          <div>Alignment Page</div>
        </Hidden>
        <Hidden on={value === 2}>
          {data ? <CustomRenderer data={data} /> : null}
        </Hidden>
      </div>
    </div>
  );
};
