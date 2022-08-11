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
  const [nCols, setNCols] = useState<number>(1);
  return (
    <Stack container style={{ padding: 20 }}>
      <Grid item>
        <Typography variant="h4">Files</Typography>
        <Button variant="contained" component="label">
          Data File
          <input hidden accept=".tsv" type="file" />
        </Button>
        <Button variant="contained" component="label">
          Alignment File
          <input hidden accept="text/csv" type="file" />
        </Button>
      </Grid>
      <Grid item container>
        <Typography variant="h4">Column Management</Typography>
        <TextField
          id="outlined-number"
          label="Number of Columns"
          type="number"
          InputLabelProps={{
            shrink: true,
          }}
          value={nCols}
          onChange={(v) =>
            parseInt(v.target.value) > 0 && setNCols(parseInt(v.target.value))
          }
        />
        <Grid item>
          {[...new Array(nCols)].map((_, i) => (
            <FormControl key={i}>
              <InputLabel id="demo-simple-select-label">
                {"Column " + (i + 1)}
              </InputLabel>
              <Select label={"Column " + (i + 1)} style={{ minWidth: 130 }}>
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
          placeholder={100}
          InputLabelProps={{
            shrink: true,
          }}
        />
      </Grid>
    </Stack>
  );
};
