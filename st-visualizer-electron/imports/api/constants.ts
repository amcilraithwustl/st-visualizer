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
  tris2Dclusters: unknown[];
  ctrs3Dclusters: unknown[];
  nClusters: number;
  ctrs2Dclusters: unknown[];
  ctrs2Dvals: [point[], [number, number][]][][]; //Slice, ctr, (points, segments)
  tris2Dvals: [point[], number[][], number[]][];
  ptValIndex: number;
  clusters: unknown[];
};

export const pointToVector = (p: point) => new THREE.Vector3(p[0], p[1], p[2]);
const getJsonData = (path: string) =>
  fetch(path, {
    headers: {
      "Content-Type": "application/json",
      Accept: "application/json",
    },
  }).then((response) => response.json());

//This is a temporary implementation and will need to be dramatically improved
export const importPts = async () => {
  try {
    const pts = (
      await import(
        "C:/Users/Aiden McIlraith/Documents/GitHub/st-visualizer/st-visualizer-electron/.webpack/main/output.json"
      )
    ).default;
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
export const defaultColor: Color = "white";
export const colors: Color[] = [
  "black",
  "red",
  "green",
  "blue",
  "yellow",
  "orange",
  "black",
  "lightgreen",
  "lightblue",
  "purple",
];
export const shrink = 0.007;
export type active = { name: string; on: boolean };

export const ipcCommands = { dialog: { openFile: "dialog:openFile" } } as const;
