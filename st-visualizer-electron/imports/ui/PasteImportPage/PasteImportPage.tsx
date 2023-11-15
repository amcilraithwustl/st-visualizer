import React from "react";
import {
  Stack,
  Typography,
  FormGroup,
  FormControlLabel,
  Checkbox,
  TextField,
  Grid,
  Button,
} from "@mui/material";
import TuneIcon from '@mui/icons-material/Tune';
import { importStateType, colTypes } from "../../api/constants";

export const PasteImportPage = ({
  importState,
  setImportState,
}: {
  importState: importStateType;
  setImportState: React.Dispatch<React.SetStateAction<importStateType>>;
}): JSX.Element => {
  return (
    <Stack style={{ padding: 20 }}>
      <Button variant="contained" component="label" startIcon={<TuneIcon />} style={{textTransform: 'none'}}>
        Paste Output (Optional) {importState.alignmentFile ? importState.alignmentFile : null}
        <input
          hidden
          multiple={false}
          accept=".csv"
          type="file"
          onChange={async (e) => {
            const file = e.target?.files?.[0];
            if (!file) return;
            const csvPath = file.path;
            const rawText = await file.text();
            const csvData = rawText.split("\n").map((l) => l.split(","));
            setImportState((s) => ({
              ...s,
              alignmentData: csvData,
              alignmentFile: csvPath,
            }));
          }}
        />
      </Button>
    </Stack>
  );
};
