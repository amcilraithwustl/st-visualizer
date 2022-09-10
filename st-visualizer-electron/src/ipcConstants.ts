import { colTypes, importStateType } from "../imports/api/constants";
import * as fs from "fs/promises";
export const ipcConstants = {
  doCalculation: "doCalculation",
  getFile: "getFile",
} as const;
import path from "path";
import { execFile } from "child_process";
import _ from "lodash";
import { app } from "electron";

//This is nodejs
export const ipcHandlers = {
  [ipcConstants.getFile]: async (
    _event: Electron.IpcMainInvokeEvent,
    { path }: { path: string }
  ) => {
    const string = await fs.readFile(path, { encoding: "utf-8" });
    return string;
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
    // const exePath = `C:/Users/Aiden McIlraith/Documents/GitHub/st-visualizer/st-visualizer/x64/Debug/st-visualizer.exe`;
    const exePath = app.isPackaged
      ? path.resolve(process.resourcesPath, "./st-visualizer.exe")
      : path.resolve(__dirname, "../../imports/static/st-visualizer.exe");

    const split = transforms.map(
      (slice) =>
        [slice.map((v) => v.x), slice.map((v) => v.y)] as [number[], number[]]
    );

    const baseLayer = split.at(0);
    if (!baseLayer) return;
    const rest = split?.slice(1);
    const alignments =
      " , , , , , ,\n" +
      rest
        ?.flatMap((r) => [r, baseLayer])
        .flatMap((rowPair) => rowPair)
        .map((row) => " , , ," + row.join(","))
        .join("\n");

    const alignmentPath = path.resolve(__dirname, "./alignments.csv");

    await fs.writeFile(alignmentPath, alignments, {
      flag: "w",
    });
    const outputPath = path.resolve(__dirname, "./output.json");
    const sliceNames = _.compact(
      _.uniq(
        importState.tsvData.map((row, i) =>
          i > 0 ? row[importState[colTypes.slice]] : undefined
        )
      )
    );
    console.log("___BEGIN PROCESS___");
    try {
      const runResults = await new Promise<string>((res, rej) => {
        execFile(
          exePath,
          [
            JSON.stringify({
              fileName: importState.fileName,
              shrink: importState.shrink,
              sliceNames: importState.sliceOrder.map((i) => sliceNames[i]),
              featureCols: importState[colTypes.feature],
              sliceIndex: importState[colTypes.slice],
              tissueIndex: importState[colTypes.tissue],
              rowIndex: importState[colTypes.row],
              colIndex: importState[colTypes.column],
              clusterIndex: importState[colTypes.cluster],
              zDistance: importState.z_distance,
            }),
            alignmentPath,
            outputPath,
          ],
          {},
          (err, data) => {
            if (err) rej(err);
            res(data);
          }
        );
      });
      console.log(runResults);
      console.log("___COMPLETE___");
      console.log("Data output to ", outputPath);

      return outputPath;
    } catch (e) {
      console.error("Running error. Please try again: ", e);
    }
  },
} as const;
