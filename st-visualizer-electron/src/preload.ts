// See the Electron documentation for details on how to use preload scripts:
// https://www.electronjs.org/docs/latest/tutorial/process-model#preload-scripts

import { contextBridge, ipcRenderer } from "electron";
import { ipcConstants, ipcHandlers } from "./ipcConstants";

const DeclaredAPI = {
  doCalculation: (args: Parameters<typeof ipcHandlers["doCalculation"]>[1]) =>
    ipcRenderer.invoke(ipcConstants.doCalculation, args) as Promise<
      ReturnType<typeof ipcHandlers["doCalculation"]>
    >,
};

contextBridge.exposeInMainWorld("electronAPI", DeclaredAPI);

declare global {
  interface Window {
    electronAPI: typeof DeclaredAPI;
  }
}
