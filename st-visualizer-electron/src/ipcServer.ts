import { ipcHandlers } from "./ipcConstants";

import { app, ipcMain } from "electron";

app.whenReady().then(() => {
  Object.entries(ipcHandlers).map((entry) =>
    ipcMain.handle(entry[0], entry[1])
  );
});
