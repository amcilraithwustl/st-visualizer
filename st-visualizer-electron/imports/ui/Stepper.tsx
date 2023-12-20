import * as React from "react";
import Stepper from "@mui/material/Stepper";
import Step from "@mui/material/Step";
import StepLabel from "@mui/material/StepLabel";
import Button from "@mui/material/Button";
import Typography from "@mui/material/Typography";
import CircularProgress from "@mui/material/CircularProgress";
import {
  AlignmentPage,
  calcTransforms,
  transformType,
} from "./AligmentPage/AlignmentPage";
import { PasteImportPage } from "./PasteImportPage/PasteImportPage";
import { NewAlignmentPage } from "./NewAlignmentPage/NewAlignmentPage";
import { ImportPage } from "./ImportPage/ImportPage";
import { SettingsPage } from "./SettingsPage/SettingsPage";
import {
  importStateType,
  datatype,
  colTypes,
  importPts,
} from "../api/constants";
import { Tooltip, Grid } from "@mui/material";
import _ from "lodash";
import { useState } from "react";
import { Settings } from "@mui/icons-material";

export const CustomStepper = ({
  importState,
  setImportState,
  setCurrentImages,
  setData,
  closeSelf,
  currentImages,
}: {
  importState: importStateType;
  setImportState: React.Dispatch<React.SetStateAction<importStateType>>;
  setCurrentImages: React.Dispatch<React.SetStateAction<transformType[]>>;
  setData: React.Dispatch<React.SetStateAction<datatype>>;
  currentImages: transformType[];
  closeSelf: () => void;
}): JSX.Element => {
  // const steps = ["Upload", "Import from Paste", "Align (Experimental)", "Align", "Settings"];
  const steps = ["Upload", "Import from Paste", "Align"];
  const [skipped, setSkipped] = React.useState(new Set<number>());
  const [activeStep, setActiveStep] = React.useState(0);

  const handleNext = () => {
    setActiveStep((prevActiveStep) => prevActiveStep + 1);
  };

  const handleBack = () => {
    setActiveStep((prevActiveStep) => prevActiveStep - 1);
  };

  const numSlices = importState.numSlices;
  const lengthsMatch = numSlices !== 0 && currentImages.length === numSlices;
  const [loading, setLoading] = useState(false);
  const stepButton =
    activeStep === steps.length ? (
      <div
        style={{
          display: "flex",
          alignItems: "center",
          justifyContent: "center",
          width: "100%",
          padding: 5
        }}
      >
        <Tooltip
          title={
            lengthsMatch
              ? "Press to calculate volumes"
              : "Alignment mismatch. " + numSlices + " slices required"
          }
        >
          <Button
            color={lengthsMatch ? "secondary" : "error"}
            onClick={() => {
              setTimeout(async () => {
                try {
                  if (!lengthsMatch) throw "Lengths Don't Match";
                  // console.log("BEGINNING");
                  setLoading(true);
                  const path = await window.electronAPI.doCalculation({
                    transforms: calcTransforms(currentImages),
                    importState,
                  });
                  if (!path) throw "No Path " + path;
                  const result = await importPts(path);
                  if (!result) throw "No Result " + result;
                  setData(result);
                } catch (e) {
                  console.error("ERROR", e);
                }
                closeSelf();
                setLoading(false);
              }, 0);
            }}
          >
            {lengthsMatch
              ? "Run Final Calculation"
              : "Run Final Calculation (" +
              currentImages.length +
              "/" +
              numSlices +
              " images uploaded)"}
          </Button>
        </Tooltip>
      </div>
    ) : (
      <div style={{ display: "flex", flexDirection: "row", padding: 5}}>
        <Button
          color="inherit"
          disabled={activeStep === 0}
          onClick={handleBack}
          sx={{ mr: 1 }}
        >
          Back
        </Button>
        <div style={{ flex: "1 1 auto" }} />
        {activeStep === steps.length - 1 ? (
          lengthsMatch ? (
            <Button onClick={handleNext}>Finish</Button>
          ) : (
            <Button onClick={handleNext} disabled>
              {"Run Final Calculation (" +
                currentImages.length +
                "/" +
                numSlices +
                " images uploaded)"}
            </Button>
          )
        ) : (
          <Button onClick={handleNext} disabled={numSlices === 0}>
            Next
          </Button>
        )}
      </div>
    );

  const stepContent = [
    <ImportPage
      key={1}
      importState={importState}
      setImportState={setImportState}
    />,
    <PasteImportPage
      key={2}
      importState={importState}
      setImportState={setImportState}
    />,
    // <NewAlignmentPage
    //   key={3}
    //   importState={importState}
    //   setImportState={setImportState}
    // />,
    <AlignmentPage
      key={3}
      setImportState={setImportState}
      currentImages={currentImages}
      setCurrentImages={setCurrentImages}
      importState={importState}
    />,
    // <SettingsPage
    //   key={5}
    //   setImportState={setImportState}
    //   importState={importState}
    // />,
  ];
  const stepLabels = steps.map((label) => {
    const stepProps: { completed?: boolean } = {};
    const labelProps: {
      optional?: React.ReactNode;
    } = {};
    return (
      <Step key={label} {...stepProps}>
        <StepLabel {...labelProps}>{label}</StepLabel>
      </Step>
    );
  });

  const finalStepper = (
    <div
      style={{
        width: "100%",
        height: "100%",
        padding: 10,
        overflowY: "auto",
        overflowX: "hidden",
        boxSizing: "border-box",
      }}
    >
      {stepButton}
      <Stepper activeStep={activeStep} style={{padding: 5}}>{stepLabels}</Stepper>
      {stepContent.at(activeStep)}
    </div>
  );

  return loading ? (
    <div
      style={{
        display: "flex",
        flexDirection: "column",
        justifyContent: "center",
        alignItems: "center",
        width: "100%",
        padding: 20,
        boxSizing: "border-box",
      }}
    >
      <CircularProgress color="secondary" />
      <Typography variant={"h6"}>Computing Geometry</Typography>
      <Typography variant={"subtitle1"}>
        Wait for it...
      </Typography>
    </div>
  ) : (
    finalStepper
  );
};
