import { Tabs, Tab } from "@mui/material";
import "react";
import * as React from "react";
import { useState } from "react";
import { datatype, importPts } from "./api/constants";
import { CustomRenderer } from "./ui/CustomRenderer";
import { ImportPage } from "./ui/ImportPage/ImportPage";

const Hidden = ({
  targetVal,
  currentVal,
  children,
}: {
  currentVal: number;
  targetVal: number;
  children: JSX.Element | null;
}) => {
  return (
    <div style={{ display: targetVal == currentVal ? undefined : "none" }}>
      {children}
    </div>
  );
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
          <Tab label="Data Display" id={1 + "tab"} />
          <Tab label="Data Export" id={2 + "tab"} />
        </Tabs>
        {value === 0 && <ImportPage />}
        <Hidden currentVal={value} targetVal={1}>
          {data ? <CustomRenderer data={data} /> : null}
        </Hidden>
      </div>
    </div>
  );
};
