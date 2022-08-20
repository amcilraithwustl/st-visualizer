import { ipcMain } from "electron";
import { importStateType } from "../imports/api/constants";
export const ipcConstants = {
  openFile: "openFile",
  doCalculation: "doCalculation",
} as const;
import path from "path";
import { execFile } from "child_process";

//This is nodejs
export const ipcHandlers = {
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
  [ipcConstants.doCalculation]: async (
    _event: Electron.IpcMainInvokeEvent,
    {
      transforms,
      importState,
    }: {
      transforms: { x: number; y: number }[][];
      importState: importStateType;
    }
  ) => {
    console.log(transforms);
    console.log(importState);
  },
} as const;
