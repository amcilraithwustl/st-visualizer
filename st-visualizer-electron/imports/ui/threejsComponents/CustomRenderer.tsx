import * as React from "react";
import * as THREE from "three";
import { Canvas, Color } from "@react-three/fiber";
import "../../api/threejsHeadeers";
import { GizmoHelper, GizmoViewport, OrbitControls } from "@react-three/drei";
import { GeometryCustom } from "./GeometryCustom";
import { Line } from "./Line";
import _ from "lodash";
import { Points } from "./Points";
import { useEffect, useState } from "react";
import {
  Checkbox,
  FormControlLabel,
  FormGroup,
  Grid,
  Stack,
} from "@mui/material";

//Display List WebGL

type datatype = {
  nat: number[];
  slices: [number, number, number][][];
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
  ctrs2Dvals: unknown[];
  tris2Dvals: unknown[];
  ptValIndex: number;
  clusters: unknown[];
};
//This is a temporary implementation and will need to be dramatically improved
const importPts = async () => {
  const pts = (await import("../../static/integrationTest.json")).default;
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

const spaceRange = 3;
const numFaces = 200 * 200;
const randomPoints = [...new Array(3 * numFaces)].map(
  () =>
    new THREE.Vector3(
      Math.random() * spaceRange - spaceRange / 2,
      Math.random() * spaceRange - spaceRange / 2,
      Math.random() * spaceRange - spaceRange / 2
    )
);
const times = 4;
export const CustomRenderer = () => {
  console.log("RENDERING " + numFaces + " " + times + " times.");
  const [data, setData] = useState<datatype | undefined>(undefined);
  useEffect(() => {
    importPts().then((res) => setData(res));
  }, []);
  const rawPtData = data?.slices.map((slice) =>
    slice.map((pt) => new THREE.Vector3(pt[0], pt[1], pt[2]))
  );
  const numPoints = rawPtData?.reduce((p, slice) => p + slice.length, 0);
  const ptsByValue =
    rawPtData &&
    data?.values
      .map((slice) =>
        slice.map((ptVals) => ptVals.indexOf(_.max(ptVals) || -1))
      )
      .map((slice, sliceIndex) =>
        slice.map((primaryValue, ptIndex) => ({
          pt: rawPtData[sliceIndex][ptIndex],
          group: primaryValue,
        }))
      )
      .slice(1, -1)
      .reduce((prev, current) => [...prev, ...current], []);

  const colors: Color[] = [
    "black",
    "red",
    "green",
    "blue",
    "yellow",
    "orange",
    "black",
    "lightgreen",
  ];
  const groups = Object.entries(_.groupBy(ptsByValue, (v) => v.group)).map(
    (entry) => ({
      group: parseInt(entry[0]) + 1,
      pts: entry[1].map((v) => v.pt),
    })
  );
  const defaultColor: Color = "transparent";

  const center = rawPtData
    ?.reduce(
      (prev, slice) =>
        slice.reduce((p, c) => p.add(c), new THREE.Vector3(0, 0, 0)),
      new THREE.Vector3(0, 0, 0)
    )
    .divideScalar(numPoints || 1);

  console.log("SLICE", rawPtData, center, numPoints);
  const translate = center
    ? ([-center.x, -center.y, -center.z] as const)
    : ([0, 0, 0] as const);
  const shrink = 0.005;
  const renderArea = (
    <Grid item>
      <Canvas style={{ height: 500 }}>
        <ambientLight />
        <gridHelper args={[100, 10]} />
        <OrbitControls makeDefault enableDamping={false} />
        <GizmoHelper
          alignment="top-right" // widget alignment within scene
          margin={[80, 80]} // widget margins (X, Y)
        >
          <GizmoViewport
            axisColors={["red", "green", "blue"]}
            labelColor="black"
          />
        </GizmoHelper>
        <pointLight position={[10, 10, 10]} />
        <pointLight position={[-10, -10, -10]} />
        {/*{rawPtData?.map((slice, i) => (*/}
        {/*  <Points*/}
        {/*    key={i}*/}
        {/*    points={slice}*/}
        {/*    color={"red"}*/}
        {/*    scale={0.01}*/}
        {/*    translate={center ? [-center.x, -center.y, -center.z] : [0, 0, 0]}*/}
        {/*  />*/}
        {/*))}*/}
        {Object.values(groups).map((g, i) => {
          console.log(activeSlices, i, activeSlices?.[i]);
          return (
            activeSlices?.[i]?.on && (
              <Points
                key={i}
                points={g.pts}
                translate={translate}
                scale={shrink}
                color={colors[g.group] || defaultColor}
              />
            )
          );
        })}
        {/*<Points points={randomPoints} color={"rgb(224,113,4)"} />*/}
        {/*<Line points={randomPoints} color={"rgb(49,222,49)"} />*/}

        {/*{[...new Array(times)].map((_, i) => (*/}
        {/*  <GeometryCustom*/}
        {/*    key={i}*/}
        {/*    points={randomPoints}*/}
        {/*    color={"rgb(153,162,255)"}*/}
        {/*    opacity={0.5}*/}
        {/*  />*/}
        {/*))}*/}

        {/*  <GeometryCustom*/}
        {/*  points={[*/}
        {/*    new THREE.Vector3(0, 0, 0),*/}
        {/*    new THREE.Vector3(10, 10, 0),*/}
        {/*    new THREE.Vector3(0, 10, 0),*/}

        {/*    new THREE.Vector3(0, 0, 0),*/}
        {/*    new THREE.Vector3(0, 10, 0),*/}
        {/*    new THREE.Vector3(10, 10, 0),*/}
        {/*  ]}*/}
        {/*  color={"rgb(153,162,255)"}*/}
        {/*  opacity={0.5}*/}
        {/*/>*/}
      </Canvas>
    </Grid>
  );
  const [activeSlices, setActiveSlices] = useState<
    { name: string; on: boolean }[]
  >([]);
  useEffect(() => {
    setActiveSlices(
      data?.sliceNames.map((v) => ({
        name: v,
        on: true,
      })) || []
    );
  }, [data?.sliceNames]);
  const controlArea = (
    <Stack>
      <FormGroup>
        {activeSlices.map((active, i) => (
          <FormControlLabel
            key={i}
            control={<Checkbox value={active.on} />}
            onChange={() => {
              const oldData = activeSlices;
              oldData[i].on = !oldData[i].on;
              setActiveSlices([...oldData]);
            }}
            label={active.name}
          />
        ))}
      </FormGroup>
    </Stack>
  );
  return (
    <Grid container>
      {controlArea}
      {renderArea}
    </Grid>
  );
};
