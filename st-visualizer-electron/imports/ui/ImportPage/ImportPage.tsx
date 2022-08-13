import {
  Button,
  FormControl,
  Grid,
  InputLabel,
  MenuItem,
  Select,
  Stack,
  TextField,
  Typography,
} from "@mui/material";
import React, { useState } from "react";

type importType = {
  sliceNames: string[]; //Names of each of the slices
  shrink: number; //Shrink factor
  alignmentFile: string; //Location of the file
  fileName: string; //Location of the file
  slice_index: number; //Index of the slice ID
  tissueIndex: number; //Index of the tissue type
  row_col_indices: [number, number]; //Location indices
  clusterIndex: number; //Index of the cluster value
  featureCols: number[]; //Columns which old features
  z_distance: number;
};
export const ImportPage = (): JSX.Element => {
  const [tsvLabels, setTSVLabels] = useState<string[]>([]);
  const nCols = tsvLabels.length;
  const [importState, setImportState] = useState<importType>({
    sliceNames: [],
    shrink: 0,
    alignmentFile: "",
    fileName: "",
    slice_index: 1,
    tissueIndex: 2,
    row_col_indices: [3, 4],
    clusterIndex: 5,
    featureCols: [],
    z_distance: 100,
  });
  const getValue = (i: number) => {
    switch (i) {
      case importState.slice_index:
        return 1;
        break;
      case importState.tissueIndex:
        return 2;
        break;
      case importState.row_col_indices[0]:
        return 3;
        break;
      case importState.row_col_indices[1]:
        return 4;
        break;
      case importState.clusterIndex:
        return 6;
        break;
      default:
        if (importState.featureCols.includes(i)) return 7;
        return null;
        break;
    }
  };

  const changeSelection = (i: number, selection: number) => {
    switch (selection) {
      case 0:
        break;
      case 1:
        setImportState((s) => ({ ...s, slice_index: i }));
        break;
      case 2:
        setImportState((s) => ({ ...s, slice_index: i }));
        break;
    }
  };
  return (
    <Stack style={{ padding: 20 }}>
      <Grid item>
        <Typography variant="h4">Files</Typography>
        <Button variant="contained" component="label">
          Data File
          <input
            hidden
            multiple={false}
            accept=".tsv"
            type="file"
            onChange={async (e) => {
              const file = e.target?.files?.[0];
              if (!file) return;
              const tsvPath = file.path;
              const rawText = await file.text();
              const tsvData = rawText.split("\n").map((l) => l.split("\t"));
              //TODO: Add guess logic here
              console.log("TSV Label Row", tsvData[0]);
              setTSVLabels(tsvData[0]);
              setImportState((s) => ({
                ...s,
                fileName: tsvPath,
                featureCols: [...new Array(tsvData[0].length - 5)].map(
                  (_, i) => i + 5
                ),
              }));
            }}
          />
        </Button>
        <Button variant="contained" component="label">
          Alignment File
          <input hidden accept="text/csv" type="file" />
        </Button>
      </Grid>
      <Grid item container>
        <Typography variant="h4">Column Management</Typography>
        <Grid item>
          {[...new Array(nCols)].map((_, i) => (
            <FormControl key={i}>
              <InputLabel id="demo-simple-select-label">
                {tsvLabels[i]}
              </InputLabel>
              <Select
                label={tsvLabels[i]}
                style={{ minWidth: 130 }}
                value={getValue(i)}
                onChange={(e) => changeSelection(i, e.target.value as number)}
              >
                <MenuItem value={0}>None</MenuItem>
                <MenuItem value={1}>Slice</MenuItem>
                <MenuItem value={2}>Tissue</MenuItem>
                <MenuItem value={3}>Row Location</MenuItem>
                <MenuItem value={4}>Column Location</MenuItem>
                <MenuItem value={6}>Cluster</MenuItem>
                <MenuItem value={7}>Feature</MenuItem>
              </Select>
            </FormControl>
          ))}
        </Grid>
      </Grid>
      <Grid item>
        <Typography variant="h4">Calculation Data</Typography>
        <TextField
          id="outlined-number"
          type="number"
          label={"Shrink (%)"}
          InputLabelProps={{
            shrink: true,
          }}
        />
        <TextField
          //Set of widths rather than a single constant one
          id="outlined-number"
          type="number"
          label={"Slice Width (Microns)"}
          value={importState.z_distance}
          InputLabelProps={{
            shrink: true,
          }}
        />
      </Grid>
    </Stack>
  );
};
