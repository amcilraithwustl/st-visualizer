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
const blankImportState = {
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
  tsvData: [] as string[][],
  z_distance: 100,
};

export type importStateType = typeof blankImportState;
export const ImportPage = (): JSX.Element => {
  const [importState, setImportState] = useState<importStateType>({
    ...blankImportState,
  });
  const tsvLabels = importState.tsvData.at(0) || [];
  const getValue = (i: number) => {
    switch (i) {
      case importState[colTypes.slice]:
        return colTypes.slice;
      case importState[colTypes.tissue]:
        return colTypes.tissue;
      case importState[colTypes.row]:
        return colTypes.row;
      case importState[colTypes.column]:
        return colTypes.column;
      case importState[colTypes.cluster]:
        return colTypes.cluster;
    }
    if (importState[colTypes.feature].includes(i)) return colTypes.feature;
    return colTypes.none;
  };
  const nCols = tsvLabels.length;

  const changeSelection = (i: number, selection: number) => {
    const currentValue = getValue(i);
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

  const columns = (
    <Grid item container>
      <Typography variant="h4">Column Management</Typography>
      <Grid item container spacing={1}>
        {[...new Array(nCols)].map((_, i) => (
          <Grid item key={i}>
            <FormControl>
              <InputLabel>{tsvLabels[i]}</InputLabel>
              <Select
                label={tsvLabels[i]}
                style={{ minWidth: 130 }}
                value={getValue(i)}
                onChange={(e) => changeSelection(i, e.target.value as number)}
              >
                <MenuItem value={colTypes.none}>
                  <em>None</em>
                </MenuItem>
                <MenuItem value={colTypes.slice}>Slice</MenuItem>
                <MenuItem value={colTypes.tissue}>Tissue</MenuItem>
                <MenuItem value={colTypes.row}>Row</MenuItem>
                <MenuItem value={colTypes.column}>Column</MenuItem>
                <MenuItem value={colTypes.cluster}>Cluster</MenuItem>
                <MenuItem value={colTypes.feature}>Feature</MenuItem>
              </Select>
            </FormControl>
          </Grid>
        ))}
      </Grid>
    </Grid>
  );
  const metadata = (
    <Grid item>
      <Typography variant="h4">Calculation Data</Typography>
      <TextField
        id="outlined-number"
        type="number"
        label={"Shrink (%)"}
        value={importState.shrink}
        onChange={(e) =>
          setImportState((s) => ({ ...s, shrink: parseInt(e.target.value) }))
        }
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
        onChange={(e) =>
          setImportState((s) => ({
            ...s,
            z_distance: parseInt(e.target.value),
          }))
        }
        InputLabelProps={{
          shrink: true,
        }}
      />
    </Grid>
  );
  return (
    <Stack style={{ padding: 20 }}>
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

            setImportState((s) => ({
              ...s,
              tsvData: tsvData,
              fileName: tsvPath,
              [colTypes.feature]: [...new Array(tsvData[0].length - 6)].map(
                (_, i) => i + 6
              ),
            }));
          }}
        />
      </Button>
      {importState.fileName ? columns : null}
      {importState.fileName ? metadata : null}
    </Stack>
  );
};
