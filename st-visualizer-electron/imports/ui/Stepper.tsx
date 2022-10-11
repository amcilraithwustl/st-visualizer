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
import { ImportPage } from "./ImportPage/ImportPage";
import {
  importStateType,
  datatype,
  colTypes,
  importPts,
} from "../api/constants";
import { Tooltip } from "@mui/material";
import _ from "lodash";
import { useState } from "react";

export function CustomStepper({
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
}) {
  const steps = ["Upload", "Align"];

  const [activeStep, setActiveStep] = React.useState(0);

  const handleNext = () => {
    setActiveStep((prevActiveStep) => prevActiveStep + 1);
  };

  const handleBack = () => {
    setActiveStep((prevActiveStep) => prevActiveStep - 1);
  };

  const slicesRow = importState.tsvData.map(
    (row) => row[importState[colTypes.slice]]
  );
  const numSlices = _.compact(_.uniq(slicesRow.slice(1))).length;

  const lengthsMatch = numSlices !== 0 && currentImages.length === numSlices;
  const [loading, setLoading] = useState(false);
  const stepButton =
    activeStep === steps.length ? (
      <>
        <Typography sx={{ mt: 2, mb: 1 }}>
          All steps completed - you&apos;re finished
        </Typography>
        <Tooltip
          title={
            lengthsMatch
              ? "Press to calculate volumes"
              : "Alignment mismatch. " + numSlices + " slices required"
          }
        >
          <Button
            color={lengthsMatch ? "secondary" : "error"}
            onClick={async () => {
              if (!lengthsMatch) return;

              console.log("BEGINNING");
              setLoading(true);
              const path = await window.electronAPI.doCalculation({
                transforms: calcTransforms(currentImages),
                importState,
              });
              if (!path) return;
              console.log("PATH", path);
              const result = await importPts(path);
              if (!result) return;
              console.log("RESULT", result);
              setData(result);
              setLoading(false);
              closeSelf();
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
      </>
    ) : (
      <>
        <Typography sx={{ marginTop: 2, marginBottom: 1 }}>
          Step {activeStep + 1}
        </Typography>
        <div style={{ display: "flex", flexDirection: "row", paddingTop: 2 }}>
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
      </>
    );

  const stepContent = [
    <ImportPage
      key={1}
      importState={importState}
      setImportState={setImportState}
    />,

    <AlignmentPage
      key={2}
      setImportState={setImportState}
      currentImages={currentImages}
      setCurrentImages={setCurrentImages}
      importState={importState}
      setData={setData}
    />,
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
        flexDirection: "column",
      }}
    >
      {stepButton}
      <Stepper activeStep={activeStep}>{stepLabels}</Stepper>
      {stepContent.at(activeStep)}
    </div>
  );
  return loading ? (
    <div
      style={{
        display: "flex",
        justifyContent: "center",
        alignContent: "center",
        width: "100%",
        padding: 10,
        boxSizing: "border-box",
      }}
    >
      <CircularProgress color="secondary" />
    </div>
  ) : (
    finalStepper
  );
}
