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
    const pts = (await import("../static/integrationTest.json")).default;
    console.log("PTS", pts);
    return (pts as unknown) as datatype;
  } catch (error) {
    console.error("File Import Error", error);
    return undefined;
  }
};

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
