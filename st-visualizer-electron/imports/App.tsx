import { Tabs, Tab } from "@mui/material";
import "react";
import * as React from "react";
import { useState } from "react";
import {
  datatype,
  importPts,
  colTypes,
  importStateType,
  blankImportState,
} from "./api/constants";
import { CustomRenderer } from "./ui/CustomRenderer";
import { AlignmentPage, transformType } from "./ui/AligmentPage/AlignmentPage";
import _ from "lodash";
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

  console.log(window.electronAPI);
  const [currentImages, setCurrentImages] = useState<transformType[]>([]);
  const [importState, setImportState] = useState<importStateType>({
    ...blankImportState,
  });
  console.log("DATA", importState);

  const slicesRow = importState.tsvData.map(
    (row) => row[importState[colTypes.slice]]
  );
  //Slice to remove the title and the compact to remove undefined
  const numSlices = _.compact(_.uniq(slicesRow.slice(1))).length;
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
          <Tab disabled={!numSlices} label="Data Alignment" id={1 + "tab"} />
          <Tab label="Data Display" id={2 + "tab"} />
        </Tabs>
        <Hidden on={value === 0}>
          <ImportPage
            importState={importState}
            setImportState={setImportState}
          />
        </Hidden>

        <Hidden on={value == 1}>
          <AlignmentPage
            setImportState={setImportState}
            currentImages={currentImages}
            setCurrentImages={setCurrentImages}
            importState={importState}
            setData={setData}
          />
        </Hidden>

        <Hidden on={value === 2}>
          {data ? <CustomRenderer data={data} /> : null}
        </Hidden>
      </div>
    </div>
  );
};
