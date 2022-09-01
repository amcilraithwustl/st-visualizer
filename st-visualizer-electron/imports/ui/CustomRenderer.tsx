import {
  Checkbox,
  FormControlLabel,
  FormGroup,
  Grid,
  Paper,
  Slider,
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
import {
  datatype,
  pointToVector,
  colors as defaultColorArray,
} from "../api/constants";
import { PointsDisplay } from "./PointsDisplay";
import { AreaDisplay } from "./AreaDisplay";
import { VolumeDisplay } from "./VolumeDisplay";
//Display List WebGL

export const CustomRenderer = ({ data }: { data: datatype }) => {
  const debounceRef = React.useRef<ReturnType<typeof setTimeout> | null>(null);
  //Data to display
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
      data?.featureNames.map((v, i) => ({
        name: v,
        on: i + 1 !== data?.featureNames.length,
      })) || []
    );
  }, [data?.featureNames]);

  const [colors, setColors] = useState(defaultColorArray);
  useEffect(() => {
    const num = activeGroups.length;
    setColors(defaultColorArray.slice(0, num));
  }, [activeGroups.length]);

  //Calculated Data
  const pointsBySlice = useMemo(
    () =>
      data?.slices.map((slice) =>
        slice.map((pt) => new THREE.Vector3(pt[0], pt[1], pt[2]))
      ),
    [data?.slices]
  );

  const pointsData = useMemo(() => {
    const ptsByValue =
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
        .reduce((prev, current) => [...prev, ...current], []);
    return Object.entries(_.groupBy(ptsByValue, (v) => v.group)).map(
      (entry) => ({
        group: parseInt(entry[0]),
        pts: entry[1].map((v) => v.pt),
      })
    );
  }, [activeSlices, data?.values, pointsBySlice]);

  const center = useMemo(() => {
    const totalNumberOfPoints = pointsBySlice?.reduce(
      (p, slice) => p + slice.length,
      0
    );
    return pointsBySlice
      ?.reduce(
        (prev, slice) =>
          prev.add(
            slice.reduce((p, c) => p.add(c), new THREE.Vector3(0, 0, 0))
          ),
        new THREE.Vector3(0, 0, 0)
      )
      .divideScalar(totalNumberOfPoints || 1);
  }, [pointsBySlice]);

  const curvesFinalData = useMemo(() => {
    if (!data) return [];
    const ctrs = data.ctrs2Dvals
      .slice(1, -1)
      .filter((_, i) => activeSlices[i]?.on)
      .map((slice) =>
        slice
          .map((ctr) => ({
            points: ctr[0].map((p) => pointToVector(p)),
            segs: ctr[1],
          }))
          .map((ctr, i) => ({
            segments: ctr.segs.flatMap((v) => {
              return v.map((v1) => ctr.points[v1]);
            }),
            val: i,
          }))
      );

    return ctrs
      .reduce(
        (acc, slice) => {
          slice.forEach((ctr) => acc[ctr.val].push(...ctr.segments));
          return acc;
        },
        [...new Array(data.nat)].map(() => [] as THREE.Vector3[])
      )
      .map((pts, group) => ({ pts, group }));
  }, [activeSlices, data]);

  const areaDisplayData = React.useMemo(() => {
    if (!data) return [];
    const ctrs = data.tris2Dvals
      .slice(1, -1)
      .filter((_, i) => activeSlices[i]?.on)
      .map((slice) => ({
        points: slice[0],
        tris: slice[1],
        vals: slice[2],
      }));

    const finalData = [...new Array(data.nat)].map(() => [] as THREE.Vector3[]);

    ctrs.flatMap((slice) =>
      slice.tris
        .map((indices, indexIndex) => ({
          triangle: indices.map((i) => pointToVector(slice.points[i])),
          val: slice.vals[indexIndex],
        }))
        .forEach((triangle) =>
          finalData[triangle.val].push(...triangle.triangle)
        )
    );
    return finalData.map((pts, group) => ({ pts, group }));
  }, [activeSlices, data]);

  const volumes = useMemo(() => {
    if (!data) return [];
    function createPolygon(poly: THREE.Vector3[]) {
      //Assuming the polygon is a star
      const centerPoint = poly
        .reduce((a, b) => a.add(b), new THREE.Vector3(0, 0, 0))
        .divideScalar(poly.length);

      return poly.flatMap((e, i) => {
        return [poly[i], poly[(i + 1) % poly.length], centerPoint];
      });
    }

    return data.ctrs3Dvals
      .map((val) => ({
        points: val[0].map((p) => pointToVector(p)),
        rings: val[1],
      }))
      .map((val) =>
        val.rings.flatMap((indices) =>
          createPolygon(indices.map((index) => val.points[index]))
        )
      )
      .map((pts, group) => ({ pts, group }));
  }, [data]);
  const [opacity, setOpacity] = useState(1);
  const data2 = useMemo(
    () => ({
      volumes: volumes,
      areaDisplayData,
      curvesFinalData,
      pointsData,
    }),
    [areaDisplayData, curvesFinalData, pointsData, volumes]
  );
  const transformedData = useMemo(
    () => ({
      volumeDisplay: data2.volumes.filter((e) => activeGroups[e.group]?.on),
      areaDisplay: data2.areaDisplayData.filter(
        (e) => activeGroups[e.group]?.on
      ),
      curvesDisplay: data2.curvesFinalData.filter(
        (e) => activeGroups[e.group]?.on
      ),
      pointsDisplay: data2.pointsData.filter((e) => activeGroups[e.group]?.on),
    }),
    [
      activeGroups,
      data2.areaDisplayData,
      data2.curvesFinalData,
      data2.pointsData,
      data2.volumes,
    ]
  );

  const pointsDisplay = center && (
    <PointsDisplay
      center={center}
      groups={transformedData.pointsDisplay}
      colors={colors}
    />
  );

  const curvesDisplay = data && center && (
    <CurvesDisplay
      center={center}
      curvesFinalData={transformedData.curvesDisplay}
      colors={colors}
    />
  );

  const areaDisplay = data && center && (
    <AreaDisplay
      center={center}
      areaDisplayData={transformedData.areaDisplay}
      colors={colors}
    />
  );

  const volumeDisplay = data && center && (
    <VolumeDisplay
      center={center}
      volumes={transformedData.volumeDisplay}
      colors={colors}
      opacity={opacity}
    />
  );

  const renderSetup = (
    <>
      <ambientLight />
      <OrbitControls makeDefault enableDamping={false} />
      <GizmoHelper
        alignment="top-right" // widget alignment within scene
        margin={[80, 80]} // widget margins (X, Y)
      >
        <GizmoViewport
          axisColors={["red", "green", "blue"]}
          labelColor="white"
        />
      </GizmoHelper>
      <pointLight position={[10, 10, 10]} />
      <pointLight position={[-10, -10, -10]} />
    </>
  );

  const renderArea = (
    <Grid item xs={12}>
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

  const primaryControlArea = (
    <Grid item container>
      <Grid item xs={3}>
        <FormGroup>
          <FormControlLabel
            control={<Checkbox checked={visuals.points} />}
            onChange={(_, checked) => {
              setVisuals((v) => ({ ...v, points: !!checked }));
            }}
            label={"Points"}
          />
        </FormGroup>
      </Grid>

      <Grid item xs={3}>
        <FormGroup>
          <FormControlLabel
            control={<Checkbox checked={visuals.contour} />}
            onChange={(_, checked) => {
              setVisuals((v) => ({ ...v, contour: !!checked }));
            }}
            label={"Contours"}
          />
        </FormGroup>
      </Grid>

      <Grid item xs={3}>
        <FormGroup>
          <FormControlLabel
            control={<Checkbox checked={visuals.area} />}
            onChange={(_, checked) => {
              setVisuals((v) => ({ ...v, area: !!checked }));
            }}
            label={"Areas"}
          />
        </FormGroup>
      </Grid>

      <Grid item xs={3}>
        <FormGroup>
          <FormControlLabel
            control={<Checkbox checked={visuals.volume} />}
            onChange={(_, checked) => {
              setVisuals((v) => ({ ...v, volume: !!checked }));
            }}
            label={"Volumes"}
          />
        </FormGroup>
      </Grid>
      <Grid item>
        <Typography>Volume Opacity</Typography>
        <Slider
          value={opacity}
          onChange={(_e, v) => setOpacity(v as number)}
          max={1}
          step={0.1}
          min={0}
        ></Slider>
      </Grid>
    </Grid>
  );

  const leftControlArea = (
    <Grid item container xs={12}>
      <Grid item>
        <Typography variant={"h5"}>Slices</Typography>
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
      </Grid>
    </Grid>
  );

  const rightControlArea = (
    <Grid item container xs={12}>
      <Grid item>
        <Typography variant={"h5"}>Groups</Typography>
        <FormGroup>
          {activeGroups.map((active, i) => (
            <>
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
              <input
                type="color"
                value={colors[i] as string}
                onChange={(e) => {
                  const c = [...colors];
                  c[i] = e.target.value;
                  if (debounceRef.current) {
                    clearTimeout(debounceRef.current);
                  }
                  debounceRef.current = setTimeout(() => setColors(c), 100);
                }}
              />
            </>
          ))}
        </FormGroup>
      </Grid>
    </Grid>
  );

  return (
    <Grid container style={{ width: "100%" }}>
      <Grid item container xs={12} spacing={3}>
        <Grid item md={3} lg={2}>
          <Paper
            style={{ width: "100%", boxSizing: "border-box", padding: 15 }}
            elevation={4}
          >
            {leftControlArea}
          </Paper>
        </Grid>

        <Grid item md={3} lg={2}>
          <Paper
            style={{ width: "100%", boxSizing: "border-box", padding: 15 }}
            elevation={4}
          >
            {rightControlArea}
          </Paper>
        </Grid>
        <Grid item container md={6} lg={8}>
          <Paper
            style={{ width: "100%", boxSizing: "border-box", padding: 15 }}
            elevation={9}
          >
            {renderArea}
            {primaryControlArea}
          </Paper>
        </Grid>
      </Grid>
    </Grid>
  );
};
