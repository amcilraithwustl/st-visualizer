// See the Electron documentation for details on how to use preload scripts:
// https://www.electronjs.org/docs/latest/tutorial/process-model#preload-scripts

import { contextBridge, ipcRenderer } from "electron";
import { ipcConstants, ipcHandlers } from "./ipcConstants";

const genHandler = <T extends keyof typeof ipcConstants>(name: T) => {
  type functionType = typeof ipcHandlers[typeof ipcConstants[T]];
  return (args: Parameters<functionType>[1]) =>
    ipcRenderer.invoke(ipcConstants[name], args) as Promise<
      ReturnType<functionType>
    >;
};

const DeclaredAPI = {
  doCalculation: genHandler("doCalculation"),
  getFile: genHandler("getFile"),
};

contextBridge.exposeInMainWorld("electronAPI", DeclaredAPI);

declare global {
  interface Window {
    electronAPI: typeof DeclaredAPI;
  }
}
