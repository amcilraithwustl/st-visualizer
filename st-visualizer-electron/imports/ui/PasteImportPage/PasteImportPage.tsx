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

export type pasteType = {
  file: File;
  data: string[][];
};

export const PasteImportPage = ({
  importState,
  setImportState,
}: {
  importState: importStateType;
  setImportState: React.Dispatch<React.SetStateAction<importStateType>>;
}): JSX.Element => {

  const numSlices = importState.numSlices;

  return (
    <Grid item container spacing={1} style={{ padding: 20 }}>
      {[...new Array(numSlices - 1)].map((_, i) => (
        <Grid item key={i} style={{width: "100%"}}>
          <Button variant="contained" component="label" startIcon={<TuneIcon />} style={{ textTransform: 'none', width: "100%" }}>
            Paste Output for Slice {i + 1} and {i + 2} {importState.pasteFiles[i+1] ? importState.pasteFiles[i+1] : ""}
            <input
              hidden
              multiple={false}
              accept=".json"
              type="file"
              onChange={async (e) => {
                const file = e.target?.files?.[0];
                if (!file) return;
                const path = file.path;
                const text = await file.text();
                const json = JSON.parse(text);
                const tmp_path = importState.pasteFiles;
                const tmp_json = importState.pasteData;
                tmp_path[i+1] = path;
                tmp_json[i+1] = json;
                setImportState((s) => ({
                  ...s,
                  pasteFiles: tmp_path,
                  pasteData: tmp_json,
                }));
              }}
            />
          </Button>
        </Grid>
      ))}
    </Grid>
  );
};
