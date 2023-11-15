import React from "react";
import { Stack, Typography, FormGroup, FormControlLabel, Checkbox, TextField, Grid } from "@mui/material";
import { importStateType, colTypes } from "../../api/constants";

export const NewAlignmentPage = ({
  importState,
  setImportState,
}: {
  importState: importStateType;
  setImportState: React.Dispatch<React.SetStateAction<importStateType>>;
}): JSX.Element => {

  return (
    <Stack style={{ padding: 20 }}>
    </Stack>
  );
}