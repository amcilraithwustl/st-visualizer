import { ipcConstants } from "./ipcConstants";

import { app, ipcMain } from "electron";

import { execFile } from "child_process";
import path from "path";

//This is nodejs
const ipcHandlers: Record<
  typeof ipcConstants[keyof typeof ipcConstants],
  Parameters<typeof ipcMain.handle>[1]
> = {
  [ipcConstants.openFile]: () => {
    return new Promise((res, rej) => {
      const exePath = path.resolve(
        __dirname,
        "../../imports/static/st-visualizer.exe"
      );
      execFile(exePath, [], {}, (err, data) => {
        console.log("ERR", err);
        console.log("DATA", data);
        if (err) rej(err);
        res(data);
      });
    });
  },
};

app.whenReady().then(() => {
  Object.entries(ipcHandlers).map((entry) =>
    ipcMain.handle(entry[0], entry[1])
  );
});
