import {
  Checkbox,
  FormControlLabel,
  FormGroup,
  Grid,
  Stack,
  Typography,
} from "@mui/material";
import { GizmoHelper, GizmoViewport, OrbitControls } from "@react-three/drei";
import { Canvas } from "@react-three/fiber";
import _ from "lodash";
import * as React from "react";
import { useEffect, useMemo, useState } from "react";
import * as THREE from "three";
import { CurvesDisplay } from "./CurvesDisplay";
import "../api/threejsHeadeers";
import { datatype, importPts } from "../api/constants";
import { PointsDisplay } from "./PointsDisplay";
import { AreaDisplay } from "./AreaDisplay";
import { VolumeDisplay } from "./VolumeDisplay";
//Display List WebGL

export const CustomRenderer = () => {
  //Data to display
  const [data, setData] = useState<datatype | undefined>(undefined);
  useEffect(() => {
    importPts().then((res) => setData(res));
  }, []);

  const [visuals, setVisuals] = useState({
    area: true,
    volume: true,
    contour: true,
    points: true,
  });

  //Display User Settings
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

  const [activeGroups, setActiveGroups] = useState<
    { name: string; on: boolean }[]
  >([]);
  useEffect(() => {
    setActiveGroups(
      data?.featureNames.map((v) => ({
        name: v,
        on: true,
      })) || []
    );
  }, [data?.featureNames]);

  //Calculated Data
  const pointsBySlice = useMemo(
    () =>
      data?.slices.map((slice) =>
        slice.map((pt) => new THREE.Vector3(pt[0], pt[1], pt[2]))
      ),
    [data?.slices]
  );

  const totalNumberOfPoints = useMemo(
    () => pointsBySlice?.reduce((p, slice) => p + slice.length, 0),
    [pointsBySlice]
  );

  const ptsByValue = useMemo(
    () =>
      pointsBySlice &&
      data?.values
        .map(
          (slice) => slice.map((ptVals) => ptVals.indexOf(_.max(ptVals) || -1)) //Get all the max indices
        )
        .map((slice, sliceIndex) =>
          slice.map((primaryValue, ptIndex) => ({
            pt: pointsBySlice[sliceIndex][ptIndex],
            group: primaryValue,
          }))
        )
        .slice(1, -1)
        .filter((v, i) => activeSlices[i]?.on)
        .reduce((prev, current) => [...prev, ...current], []),
    [activeSlices, data?.values, pointsBySlice]
  );

  const groups = useMemo(
    () =>
      Object.entries(_.groupBy(ptsByValue, (v) => v.group)).map((entry) => ({
        group: parseInt(entry[0]) + 1,
        pts: entry[1].map((v) => v.pt),
      })),
    [ptsByValue]
  );

  const center = useMemo(
    () =>
      pointsBySlice
        ?.reduce(
          (prev, slice) =>
            prev.add(
              slice.reduce((p, c) => p.add(c), new THREE.Vector3(0, 0, 0))
            ),
          new THREE.Vector3(0, 0, 0)
        )
        .divideScalar(totalNumberOfPoints || 1),
    [totalNumberOfPoints, pointsBySlice]
  );

  const controlArea = (
    <Grid item xs={4}>
      <Stack>
        <Typography>Slices</Typography>
        <FormGroup>
          {activeSlices.map((active, i) => (
            <FormControlLabel
              key={i}
              control={<Checkbox checked={active.on} />}
              onChange={(_, checked) => {
                const oldData = activeSlices;
                oldData[i].on = checked;

                setActiveSlices([...oldData]);
              }}
              label={active.name}
            />
          ))}
        </FormGroup>

        <Typography>Groups</Typography>
        <FormGroup>
          {activeGroups.map((active, i) => (
            <FormControlLabel
              key={i}
              control={<Checkbox checked={active.on} />}
              onChange={(_, checked) => {
                const oldData = activeGroups;
                oldData[i].on = checked;

                setActiveGroups([...oldData]);
              }}
              label={active.name}
            />
          ))}
        </FormGroup>

        <Typography>Visualizations</Typography>
        <FormGroup>
          <FormControlLabel
            control={<Checkbox checked={visuals.points} />}
            onChange={(_, checked) => {
              setVisuals((v) => ({ ...v, points: !!checked }));
            }}
            label={"Points"}
          />

          <FormControlLabel
            control={<Checkbox checked={visuals.contour} />}
            onChange={(_, checked) => {
              setVisuals((v) => ({ ...v, contour: !!checked }));
            }}
            label={"Contours"}
          />

          <FormControlLabel
            control={<Checkbox checked={visuals.area} />}
            onChange={(_, checked) => {
              setVisuals((v) => ({ ...v, area: !!checked }));
            }}
            label={"Areas"}
          />

          <FormControlLabel
            control={<Checkbox checked={visuals.volume} />}
            onChange={(_, checked) => {
              setVisuals((v) => ({ ...v, volume: !!checked }));
            }}
            label={"Volumes"}
          />
        </FormGroup>
      </Stack>
    </Grid>
  );

  const pointsDisplay = center && (
    <PointsDisplay
      activeGroups={activeGroups}
      center={center}
      groups={groups}
    />
  );

  const curvesDisplay = data && center && (
    <CurvesDisplay
      ctrs2Dvals={data.ctrs2Dvals}
      center={center}
      activeGroups={activeGroups}
      activeSlices={activeSlices}
      nVals={data.nat}
    />
  );

  const areaDisplay = data && center && (
    <AreaDisplay
      data={data}
      center={center}
      activeGroups={activeGroups}
      activeSlices={activeSlices}
      nVals={data.nat}
    />
  );

  const volumeDisplay = data && center && (
    <VolumeDisplay data={data} center={center} activeGroups={activeGroups} />
  );

  const renderSetup = (
    <>
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
    </>
  );

  const renderArea = (
    <Grid item xs={8}>
      <Canvas
        style={{
          height: 500,
          width: "100%",
          borderStyle: "solid",
          borderColor: "black",
          borderWidth: 3,
        }}
      >
        {renderSetup}

        {visuals.points && pointsDisplay}
        {visuals.contour && curvesDisplay}
        {visuals.area && areaDisplay}
        {visuals.volume && volumeDisplay}
      </Canvas>
    </Grid>
  );
  return (
    <Grid container style={{ width: "100%" }}>
      {controlArea}
      {renderArea}
    </Grid>
  );
};
