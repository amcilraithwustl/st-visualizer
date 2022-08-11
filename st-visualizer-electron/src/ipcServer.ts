import { ipcConstants } from "./ipcConstants";

import { app, ipcMain } from "electron";

//This is nodejs
const ipcHandlers: Record<
  typeof ipcConstants[keyof typeof ipcConstants],
  Parameters<typeof ipcMain.handle>[1]
> = {
  [ipcConstants.openFile]: () => {
    console.log("HELLO WORLD SERVER");
    console.log("PROCESS NAME", process);
    return "RETURNED VALUE";
  },
};

app.whenReady().then(() => {
  Object.entries(ipcHandlers).map((entry) =>
    ipcMain.handle(entry[0], entry[1])
  );
});
