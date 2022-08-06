// See the Electron documentation for details on how to use preload scripts:
// https://www.electronjs.org/docs/latest/tutorial/process-model#preload-scripts

import { contextBridge } from "electron";

export interface IElectronAPI {
  loadPreferences: () => Promise<void>;
}

console.log("EXPOSING IN MAIN WORLD");

contextBridge.exposeInMainWorld("electronAPI", {
  doAThing: () => {
    console.log("Hello world!");
  },
});

declare global {
  interface Window {
    electronAPI: IElectronAPI;
  }
}
