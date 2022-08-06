import { ipcConstants } from "./ipcConstants";

import { app, ipcMain } from "electron";

app.whenReady().then(() => {
  ipcMain.handle(ipcConstants.openFile, () => {
    console.log("HELLO WORLD SERVER");
    console.log("PROCESS NAME", process);
    return "RETURNED VALUE";
  });
});
