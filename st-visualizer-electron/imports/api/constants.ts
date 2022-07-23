import * as THREE from "three";
import { Color } from "@react-three/fiber";

type point = [number, number, number];
export type datatype = {
  nat: number[];
  slices: point[][];
  shrink: number[];
  sliceNames: string[];
  values: number[][][];
  featureNames: string[];
  featureCols: number[];
  ptClusIndex: number;
  ctrs3Dvals: unknown[];
  tris2Dclusters: unknown[];
  ctrs3Dclusters: unknown[];
  nClusters: number;
  ctrs2Dclusters: unknown[];
  ctrs2Dvals: [point[], [number, number][]][][]; //Slice, ctr, (points, segments)
  tris2Dvals: unknown[];
  ptValIndex: number;
  clusters: unknown[];
};

export const pointToVector = (p: point) => new THREE.Vector3(p[0], p[1], p[2]);

//This is a temporary implementation and will need to be dramatically improved
export const importPts = async () => {
  const pts = (await import("../static/integrationTest.json")).default;
  console.log("PTS", pts);
  return (pts as unknown) as datatype;

  // ret.push_back(results.values[0][0].size()); //nMat,
  // ret.push_back(shrink); //shrink,
  // ret.push_back(results.clusters); //clusters,
  // ret.push_back(slices); //slices,
  // ret.push_back(ptClusIndex); //ptClusIndex,
  // ret.push_back(convertCtrs(ctrs2dVals)); //ctrs2Dvals,
  // ret.push_back(convert3D(ctrs3dVals)); //ctrs3Dvals,
  // ret.push_back(results.names); //featureNames,
  // ret.push_back(ptValIndex); //ptValIndex,
  // ret.push_back(convertTris(tris2dVals)); //tris2Dvals
  // ret.push_back(convertCtrs(ctrs2dclusters)); //ctrs2Dclusters,
  // ret.push_back(convert3D(ctrs3dClusters)); //ctrs3Dclusters,
  // ret.push_back(results.clusters[0][0].size()); //nClusters,
  // ret.push_back(convertTris(tris2dclusters)); //tris2Dclusters,
  // ret.push_back(featureCols); //featureCols,
  //sliceNames
  //values
};

export const times = 4;
export const defaultColor: Color = "transparent";
export const colors: Color[] = [
  "black",
  "red",
  "green",
  "blue",
  "yellow",
  "orange",
  "black",
  "lightgreen",
];
export const shrink = 0.007;
export type active = { name: string; on: boolean };
