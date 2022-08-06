// See the Electron documentation for details on how to use preload scripts:
// https://www.electronjs.org/docs/latest/tutorial/process-model#preload-scripts

import { contextBridge, ipcRenderer } from "electron";
import { ipcConstants } from "./ipcConstants";

const DeclaredAPI = {
  doAThing: () => {
    console.log("Hello world!");
  },
  basicInvoke: () => ipcRenderer.invoke(ipcConstants.openFile),
};

contextBridge.exposeInMainWorld("electronAPI", DeclaredAPI);

declare global {
  interface Window {
    electronAPI: typeof DeclaredAPI;
  }
}
