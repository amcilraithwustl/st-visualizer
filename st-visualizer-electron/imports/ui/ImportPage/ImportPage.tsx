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

enum colTypes {
  none,
  slice,
  tissue,
  row,
  column,
  cluster,
  feature,
}
export const ImportPage = (): JSX.Element => {
  const [tsvLabels, setTSVLabels] = useState<string[]>([]);
  const nCols = tsvLabels.length;
  const [importState, setImportState] = useState({
    sliceNames: [],
    shrink: 0,
    alignmentFile: "",
    fileName: "",
    [colTypes.slice]: 1,
    [colTypes.tissue]: 2,
    [colTypes.row]: 3,
    [colTypes.column]: 4,
    [colTypes.cluster]: 5,
    [colTypes.feature]: [] as number[],
    z_distance: 100,
  });
  const getValue = (i: number) => {
    switch (i) {
      case importState[colTypes.slice]:
        return colTypes.slice;
        break;
      case importState[colTypes.tissue]:
        return colTypes.tissue;
        break;
      case importState[colTypes.row]:
        return colTypes.row;
        break;
      case importState[colTypes.column]:
        return colTypes.column;
        break;
      case importState[colTypes.cluster]:
        return colTypes.cluster;
        break;
    }
    if (importState[colTypes.feature].includes(i)) return colTypes.feature;
    return colTypes.none;
  };

  const changeSelection = (i: number, selection: number) => {
    const currentValue = getValue(i);
    console.log(i, selection, currentValue);

    if (currentValue === colTypes.feature) {
      setImportState((s) => ({
        ...s,
        [colTypes.feature]: s[colTypes.feature].filter((v) => v !== i),
      }));
    } else {
      setImportState((s) => ({ ...s, [currentValue]: -1 }));
    }

    if (selection === colTypes.feature) {
      setImportState((s) => ({
        ...s,
        [colTypes.feature]: [...s[colTypes.feature], i],
      }));
    } else {
      setImportState((s) => ({ ...s, [selection]: i }));
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
                [colTypes.feature]: [...new Array(tsvData[0].length - 6)].map(
                  (_, i) => i + 6
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
        <Grid item container spacing={3}>
          {[...new Array(nCols)].map((_, i) => (
            <Grid item key={i}>
              <FormControl>
                <InputLabel id="demo-simple-select-label">
                  {tsvLabels[i]}
                </InputLabel>
                <Select
                  label={tsvLabels[i]}
                  style={{ minWidth: 130 }}
                  value={getValue(i)}
                  onChange={(e) => changeSelection(i, e.target.value as number)}
                >
                  <MenuItem value={colTypes.none}>None</MenuItem>
                  <MenuItem value={colTypes.slice}>Slice</MenuItem>
                  <MenuItem value={colTypes.tissue}>Tissue</MenuItem>
                  <MenuItem value={colTypes.row}>Row Location</MenuItem>
                  <MenuItem value={colTypes.column}>Column Location</MenuItem>
                  <MenuItem value={colTypes.cluster}>Cluster</MenuItem>
                  <MenuItem value={colTypes.feature}>Feature</MenuItem>
                </Select>
              </FormControl>
            </Grid>
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
