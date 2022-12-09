import * as THREE from "three";
import { Color } from "@react-three/fiber";

type point = [number, number, number];
export type datatype = {
  nat: number;
  slices: point[][];
  shrink: number[];
  sliceNames: string[];
  values: number[][][];
  featureNames: string[];
  featureCols: number[];
  ptClusIndex: number;
  ctrs3Dvals: [point[], number[][]][]; //Value, [points, point indices per face]
  ctrs2Dvals: [point[], [number, number][]][][]; //Slice, ctr, (points, segments)
  tris2Dvals: [point[], number[][], number[]][];
  ptValIndex: number;
  nClusters: number;
  clusters: number[][][];
  ctrs3Dclusters: [point[], number[][]][];
  ctrs2Dclusters: [point[], [number, number][]][][];
  tris2Dclusters: [point[], number[][], number[]][];
};

export const pointToVector = (p: point) => new THREE.Vector3(p[0], p[1], p[2]);
const getJsonData = async (path: string) => {
  const buffer = await window.electronAPI.getFile({ path });
  console.log("STRING", buffer);
  return JSON.parse(buffer.toString());
};
//This is a temporary implementation and will need to be dramatically improved
export const importPts = async (path: string) => {
  try {
    console.log("TRYING TO DO JSON");
    const pts = await getJsonData(path);
    console.log("PTS", pts);
    return (pts as unknown) as datatype;
  } catch (error) {
    console.error("File Import Error", error);
    return undefined;
  }
};

export enum colTypes {
  none,
  slice,
  tissue,
  row,
  column,
  cluster,
  feature,
}
export const blankImportState = {
  sliceOrder: [] as number[],
  shrink: 0,
  alignmentFile: "",
  fileName: "",
  [colTypes.slice]: 1,
  [colTypes.tissue]: 2,
  [colTypes.row]: 3,
  [colTypes.column]: 4,
  [colTypes.cluster]: 5,
  [colTypes.feature]: [] as number[],
  tsvData: [] as string[][],
  z_distance: 100,
};

export const getValue = (importState: importStateType, i: number) => {
  switch (i) {
    case importState[colTypes.slice]:
      return colTypes.slice;
    case importState[colTypes.tissue]:
      return colTypes.tissue;
    case importState[colTypes.row]:
      return colTypes.row;
    case importState[colTypes.column]:
      return colTypes.column;
    case importState[colTypes.cluster]:
      return colTypes.cluster;
  }
  if (importState[colTypes.feature].includes(i)) return colTypes.feature;
  return colTypes.none;
};

export type importStateType = typeof blankImportState;

export const times = 4;
export const defaultColor: Color = "#FFFFFF";

//Good for 90 inputs before it runs out
export const colors: Color[] = [...new Array(10)].flatMap(() => [
  "#000000",
  "#000080",
  "#0000FF",
  "#808000",
  "#FF0000",
  "#0099FF",
  "#008000",
  "#AA0000",
  "#FF00FF",
  "#FFFF00",
]);
export const shrink = 0.007;
export type active = { name: string; on: boolean };

export const ipcCommands = { dialog: { openFile: "dialog:openFile" } } as const;

export const customFileExtension = ".stvis";
