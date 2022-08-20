import { Tabs, Tab, Button, Tooltip } from "@mui/material";
import "react";
import * as React from "react";
import { useState } from "react";
import { datatype, importPts } from "./api/constants";
import { CustomRenderer } from "./ui/CustomRenderer";
import { AlignmentPage, transformType } from "./ui/AligmentPage/AlignmentPage";
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
  const [currentImages, setCurrentImages] = useState<transformType[]>([]);
  const lengthsMatch = currentImages.length === data?.slices.length;
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
          {data ? <Tab label="Data Alignment" id={1 + "tab"} /> : null}
          <Tab label="Data Display" id={2 + "tab"} />
        </Tabs>
        <Hidden on={value === 0}>
          <ImportPage />
        </Hidden>
        <Hidden on={value == 1}>
          <Tooltip
            title={
              lengthsMatch
                ? "Press to calculate volumes"
                : "Alignment mismatch. " +
                  data?.slices.length +
                  " slices required"
            }
          >
            <Button color={lengthsMatch ? "secondary" : "error"}>
              Run Final Calculation{lengthsMatch ? "" : " (Not Ready)"}
            </Button>
          </Tooltip>
        </Hidden>
        <Hidden on={value == 1}>
          <AlignmentPage
            currentImages={currentImages}
            setCurrentImages={setCurrentImages}
          />
        </Hidden>

        <Hidden on={value === 2}>
          {data ? <CustomRenderer data={data} /> : null}
        </Hidden>
      </div>
    </div>
  );
};
