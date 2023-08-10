import React from "react";
import { Stack, Typography, FormGroup, FormControlLabel, Checkbox, TextField, Grid } from "@mui/material";
import { importStateType, colTypes } from "../../api/constants";

export const SettingsPage = ({
    importState,
    setImportState,
}: {
    importState: importStateType;
    setImportState: React.Dispatch<React.SetStateAction<importStateType>>;
}): JSX.Element => {

    const [exportToggle, exportToggleSet] = React.useState(false);
    const pathTextFields = (
        <Stack>
            <TextField label="Path for features" />
            <TextField label="Path for clusters" />
        </Stack>
    );

    return (
        <Stack style={{ padding: 20 }}>
            <Typography variant="h4">Settings</Typography>
            <FormGroup>
                <FormControlLabel
                    control={<Checkbox onChange={() => exportToggleSet(!exportToggle)} />}
                    label="Export to obj files"
                />
                {exportToggle ? pathTextFields : <></>}
            </FormGroup>
        </Stack>
    );
}