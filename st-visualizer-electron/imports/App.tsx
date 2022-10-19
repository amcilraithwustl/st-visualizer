import { Tabs, Tab, Dialog, Button } from "@mui/material";
import "react";
import * as React from "react";
import { useState } from "react";
import { CustomStepper } from "./ui/Stepper";
import { datatype, importStateType, blankImportState } from "./api/constants";
import { CustomRenderer } from "./ui/CustomRenderer";
import { transformType } from "./ui/AligmentPage/AlignmentPage";
//TODO: Shading
//TODO: More visualization combinations
export const App = () => {
  const [data, setData] = useState<datatype | undefined>(undefined);

  console.log(window.electronAPI);
  const [currentImages, setCurrentImages] = useState<transformType[]>([]);
  const [importState, setImportState] = useState<importStateType>({
    ...blankImportState,
  });
  console.log("DATA", importState);

  //Slice to remove the title and the compact to remove undefined
  const [isOpen, setIsOpen] = useState(false);
  const dialog = (
    <Dialog
      open={isOpen}
      onClose={() => setIsOpen(false)}
      fullWidth
      maxWidth={"xl"}
    >
      <CustomStepper
        setImportState={setImportState}
        currentImages={currentImages}
        setCurrentImages={setCurrentImages}
        importState={importState}
        setData={setData}
        closeSelf={() => setIsOpen(false)}
      />
    </Dialog>
  );
  return (
    <div
      style={{
        padding: 10,
        boxSizing: "border-box",
        width: "100%",
        height: "100%",
      }}
    >
      {dialog}
      <div style={{ width: "100%" }}>
        <CustomRenderer
          setIsOpen={() => setIsOpen(true)}
          data={data || undefined}
          setData={setData}
        />
      </div>
    </div>
  );
};
