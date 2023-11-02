import {
  Button,
  Checkbox,
  FormControlLabel,
  FormGroup,
  Grid,
  Tab,
  Box,
  Stack,
  Paper,
  Slider,
  Switch,
  Typography,
} from "@mui/material";
import { TabContext, TabList, TabPanel } from '@mui/lab';
import { DataGrid, GridColDef, GridValueGetterParams, GridRenderCellParams, GridRowSelectionModel, GridValueFormatterParams } from '@mui/x-data-grid';
import { GizmoHelper, GizmoViewport, OrbitControls } from "@react-three/drei";
import { Canvas, useThree } from "@react-three/fiber";
import _, { toInteger } from "lodash";
import * as React from "react";
import { useCallback, useEffect, useMemo, useRef, useState } from "react";
import * as THREE from "three";
import { CurvesDisplay } from "./CurvesDisplay";
import "../api/threejsHeadeers";
import {
  datatype,
  pointToVector,
  colors as defaultColorArray,
  customFileExtension,
} from "../api/constants";
import { PointsDisplay } from "./PointsDisplay";
import { AreaDisplay } from "./AreaDisplay";
import { VolumeDisplay } from "./VolumeDisplay";
//Display List WebGL

const saveFileGeneric = (href: string, filename: string) => {
  // create "a" HTML element with href to file
  const link = document.createElement("a");
  link.href = href;
  link.download = filename;
  document.body.appendChild(link);
  link.click();

  // clean up "a" element & remove ObjectURL
  document.body.removeChild(link);
  URL.revokeObjectURL(href);
};

//Modified from https://stackoverflow.com/questions/55613438/reactwrite-to-json-file-or-export-download-no-server
const saveFile = (myData: Record<string, unknown>) => {
  // create file in browser
  const fileName = "savedata";
  const json = JSON.stringify(myData); //, null, 2);
  const blob = new Blob([json], { type: "application/json" });
  const href = URL.createObjectURL(blob);

  saveFileGeneric(href, fileName + customFileExtension);
};

const uid = Math.random().toString();

const Screengrabber = ({
  passthrough,
}: {
  passthrough: (takeScreenshot: () => string) => void;
}) => {
  const { gl, scene, camera } = useThree();

  const takeScreenshot = useCallback(() => {
    gl.render(scene, camera);
    return gl.domElement.toDataURL();
  }, [camera, gl, scene]);
  useEffect(() => passthrough(takeScreenshot), [takeScreenshot, passthrough]);
  return <mesh />;
};

export const CustomRenderer = ({
  data: data,
  setData,
  setIsOpen,
}: {
  data: datatype | undefined;
  setData: React.Dispatch<React.SetStateAction<datatype | undefined>>;
  setIsOpen: () => void;
}) => {
  const debounceRef = React.useRef<ReturnType<typeof setTimeout> | null>(null);
  //Data to display
  const [visuals, setVisuals] = useState({
    area: true,
    volume: true,
    contour: true,
    points: true,
  });

  const [doClusters, setDoClusters] = useState(false);

  const sliceNames = useMemo(
    () => (doClusters ? data?.sliceNames : data?.sliceNames),
    [data?.sliceNames, doClusters]
  );
  const slices = useMemo(() => (doClusters ? data?.slices : data?.slices), [
    data?.slices,
    doClusters,
  ]);
  const ctrs2Dvals = useMemo(
    () => (doClusters ? data?.ctrs2Dclusters : data?.ctrs2Dvals),
    [data?.ctrs2Dclusters, data?.ctrs2Dvals, doClusters]
  );
  const numFeatures = useMemo(
    () => (doClusters ? data?.nClusters : data?.nat),
    [data?.nClusters, data?.nat, doClusters]
  );
  const tris2Dvals = useMemo(
    () => (doClusters ? data?.tris2Dclusters : data?.tris2Dvals),
    [data?.tris2Dclusters, data?.tris2Dvals, doClusters]
  );
  const ctrs3Dvals = useMemo(
    () => (doClusters ? data?.ctrs3Dclusters : data?.ctrs3Dvals),
    [data?.ctrs3Dclusters, data?.ctrs3Dvals, doClusters]
  );
  const featureNames = useMemo(
    () =>
      doClusters
        ? [...new Array(data?.nClusters)].map((v, i) => "" + i)
        : data?.featureNames,
    [data?.featureNames, data?.nClusters, doClusters]
  );
  const values = useMemo(() => (doClusters ? data?.clusters : data?.values), [
    data?.clusters,
    data?.values,
    doClusters,
  ]);

  // FIXME: memeory leak

  // const ctrsSurfaceAreaClusters = data?.ctrsSurfaceAreaClusters.map(x => x.toLocaleString('en-US', { minimumFractionDigits: 2, maximumFractionDigits: 2 }));
  // const ctrsSurfaceAreaVals = data?.ctrsSurfaceAreaVals.map(x => x.toLocaleString('en-US', { minimumFractionDigits: 2, maximumFractionDigits: 2 }));
  // const ctrsVolumeClusters = data?.ctrsVolumeClusters.map(x => x.toLocaleString('en-US', { minimumFractionDigits: 2, maximumFractionDigits: 2 }));
  // const ctrsVolumeVals = data?.ctrsVolumeVals.map(x => x.toLocaleString('en-US', { minimumFractionDigits: 2, maximumFractionDigits: 2 }));

  // const surfaceArea = useMemo(() => (doClusters ? ctrsSurfaceAreaClusters : ctrsSurfaceAreaVals), [ctrsSurfaceAreaClusters, ctrsSurfaceAreaVals, doClusters]);
  // const volume = useMemo(() => (doClusters ? ctrsVolumeClusters : ctrsVolumeVals), [ctrsVolumeClusters, ctrsVolumeVals, doClusters]);

  const surfaceArea = useMemo(() => (doClusters ? data?.ctrsSurfaceAreaClusters : data?.ctrsSurfaceAreaVals), [data?.ctrsSurfaceAreaClusters, data?.ctrsSurfaceAreaVals, doClusters]);
  const volume = useMemo(() => (doClusters ? data?.ctrsVolumeClusters : data?.ctrsVolumeVals), [data?.ctrsVolumeClusters, data?.ctrsVolumeVals, doClusters]);

  const components = useMemo(() => (doClusters ? data?.componentsClusters : data?.componentsVals), [data?.componentsClusters, data?.componentsVals, doClusters]);
  const handles = useMemo(() => (doClusters ? data?.handlesClusters : data?.handlesVals), [data?.handlesClusters, data?.handlesVals, doClusters]);

  //Display User Settings
  const [activeSlices, setActiveSlices] = useState<
    { name: string; on: boolean }[]
  >([]);
  useEffect(() => {
    setActiveSlices(
      sliceNames?.map((v) => ({
        name: v,
        on: true,
      })) || []
    );
  }, [sliceNames]);

  const [activeGroups, setActiveGroups] = useState<
    { name: string; on: boolean; area: number; volume: number; components: number; handles: number; }[]
  >([]);
  useEffect(() => {
    setActiveGroups(
      featureNames?.map((v, i) => ({
        name: i + 1 !== featureNames.length ? v : "No Tissue",
        on: i + 1 !== featureNames.length,
        area: surfaceArea ? surfaceArea[i] : 0,
        volume: volume ? volume[i] : 0,
        components: components ? components[i] : 0,
        handles: handles ? handles[i] : 0,
      })) || []
    );
  }, [featureNames, surfaceArea, volume, components, handles]);

  const [colors, setColors] = useState(defaultColorArray);
  const [opacity, setOpacity] = useState(1);
  useEffect(() => {
    const num = activeGroups.length;
    setColors(defaultColorArray.slice(0, num));
  }, [activeGroups.length]);

  const camRef = useRef(null);
  const saveHandler = useCallback(() => {
    const d = {
      activeGroups,
      activeSlices,
      colors,
      data,
      visuals,
      opacity,
      doClusters,
    };
    saveFile(d);
  }, [activeGroups, activeSlices, colors, data, opacity, visuals, doClusters]);

  useEffect(() => {
    const stop = window.electronAPI.onSave(saveHandler);
    return () => {
      stop();
    };
  }, [saveHandler]);

  const openExisting = (
    <Button variant="contained" component="label" id={uid}>
      Open Geometry
      <input
        hidden
        multiple={false}
        accept={customFileExtension}
        type="file"
        onChange={async (e) => {
          const target = e.currentTarget as HTMLInputElement;
          const files = target?.files;
          const file = files?.[0];
          if (!file) return;
          const rawText = await file.text();
          const d = JSON.parse(rawText);
          setData(d.data || data);
          console.log(d);
          setTimeout(() => {
            //Placeholder to deal with sideffects of data change
            setActiveGroups(d.activeGroups || activeGroups);
            setActiveSlices(d.activeSlices || activeSlices);
            setColors(d.colors || colors);
            setVisuals(d.visuals || visuals);
            setOpacity(d.opacity !== undefined ? d.opacity : opacity);
            setDoClusters(!!d.doClusters);
          }, 1000);
        }}
      />
    </Button>
  );

  useEffect(() => {
    const openHandler = () => {
      document.getElementById(uid)?.click();
    };
    const stop = window.electronAPI.onOpen(openHandler);
    return () => {
      stop();
    };
  }, []);
  //Calculated Data
  const pointsBySlice = useMemo(
    () =>
      slices?.map((slice) =>
        slice.map((pt) => new THREE.Vector3(pt[0], pt[1], pt[2]))
      ),
    [slices]
  );

  const pointsData = useMemo(() => {
    const ptsByValue =
      pointsBySlice &&
      values
        ?.map(
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
  }, [activeSlices, values, pointsBySlice]);

  const center = useMemo(() => {
    if (!pointsBySlice) return undefined;
    const totalNumberOfPoints = pointsBySlice.reduce(
      (p, slice) => p + slice.length,
      0
    );
    return pointsBySlice
      .reduce(
        (prev, slice) =>
          prev.add(
            slice.reduce((p, c) => p.add(c), new THREE.Vector3(0, 0, 0))
          ),
        new THREE.Vector3(0, 0, 0)
      )
      .divideScalar(totalNumberOfPoints || 1);
  }, [pointsBySlice]);

  const curvesFinalData = useMemo(() => {
    if (!ctrs2Dvals) return [];
    const ctrs = ctrs2Dvals
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
        [...new Array(numFeatures)].map(() => [] as THREE.Vector3[])
      )
      .map((pts, group) => ({ pts, group }));
  }, [activeSlices, ctrs2Dvals, numFeatures]);
  const areaDisplayData = React.useMemo(() => {
    if (!tris2Dvals) return [];
    const ctrs = tris2Dvals
      .slice(1, -1)
      .filter((_, i) => activeSlices[i]?.on)
      .map((slice) => ({
        points: slice[0],
        tris: slice[1],
        vals: slice[2],
      }));

    const finalData = [...new Array(numFeatures)].map(
      () => [] as THREE.Vector3[]
    );

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
  }, [activeSlices, numFeatures, tris2Dvals]);

  const volumes = useMemo(() => {
    function createPolygon(poly: THREE.Vector3[]) {
      //Assuming the polygon is a star
      const centerPoint = poly
        .reduce((a, b) => a.add(b), new THREE.Vector3(0, 0, 0))
        .divideScalar(poly.length);

      return poly.flatMap((e, i) => {
        return [poly[i], poly[(i + 1) % poly.length], centerPoint];
      });
    }
    if (!ctrs3Dvals) return [];
    return ctrs3Dvals
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
  }, [ctrs3Dvals]);
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
      opacity={opacity}
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

  const canvas = useRef<(() => string) | null>(null);

  const saveCanvas = React.useCallback(() => {
    if (!canvas.current) return;
    const url = canvas.current();
    url && saveFileGeneric(url, "image");
  }, []);

  const renderSetup = (
    <>
      <Screengrabber passthrough={(v) => (canvas.current = v)} />
      <ambientLight />
      <OrbitControls makeDefault enableDamping={false} ref={camRef} />
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
    <Canvas
      style={{
        // FIXME: why can't i set height to 100%? 
        height: "80vh",
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
  );

  const viewControl = (
    <Grid item container>
      <Grid item xs={12}>
        <FormGroup>
          <FormControlLabel
            control={<Switch checked={visuals.points} />}
            onChange={(_, checked) => {
              setVisuals((v) => ({ ...v, points: !!checked }));
            }}
            label={"Points"}
          />
          <FormControlLabel
            control={<Switch checked={visuals.contour} />}
            onChange={(_, checked) => {
              setVisuals((v) => ({ ...v, contour: !!checked }));
            }}
            label={"Contours"}
          />
          <FormControlLabel
            control={<Switch checked={visuals.area} />}
            onChange={(_, checked) => {
              setVisuals((v) => ({ ...v, area: !!checked }));
            }}
            label={"Areas"}
          />
          <FormControlLabel
            control={<Switch checked={visuals.volume} />}
            onChange={(_, checked) => {
              setVisuals((v) => ({ ...v, volume: !!checked }));
            }}
            label={"Volumes"}
          />
        </FormGroup>
      </Grid>

      <Grid item style={{ paddingTop: 15 }}>
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

  const sliceControl = (
    <Grid item container xs={12}>
      <FormGroup>
        {activeSlices.map((active, i) => (
          <FormControlLabel
            key={i}
            control={<Switch checked={active.on} />}
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
  );

  const groupControl = (
    <Grid item container xs={12}>
      <Grid item>
        <FormGroup>
          {activeGroups.map((active, i) => (
            <div
              key={i}
              style={{
                display: "flex",
                alignItems: "center",
                width: "100%",
              }}
            >
              <div
                style={{
                  width: 80,
                  display: "flex",
                  alignItems: "center",
                  justifyContent: "space-around",
                  paddingRight: 10,
                }}
              >
                <Checkbox
                  checked={active.on}
                  onChange={(_, checked) => {
                    const oldData = activeGroups;
                    oldData[i].on = checked;
                    setActiveGroups([...oldData]);
                  }}
                />
                <input
                  style={{ width: 30 }}
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
              </div>
              <div style={{ flex: 1 }}>
                <Typography>{active.name}</Typography>
              </div>
            </div>
          ))}
        </FormGroup>
      </Grid>
    </Grid>
  );

  const columns: GridColDef[] = [
    { field: 'id', headerName: '#', width: 30 },
    {
      field: 'color',
      headerName: 'Color',
      width: 30,
      description: 'Pick your color here',
      renderCell: (props: GridRenderCellParams) => (
        <input
          style={{ width: 30 }}
          type="color"
          value={colors[props.row.id] as string}
          onChange={async (e) => {
            const c = [...colors];
            c[props.row.id] = e.target.value;
            if (debounceRef.current) {
              clearTimeout(debounceRef.current);
            }
            debounceRef.current = setTimeout(() => setColors(c), 100);
          }}
        />
      )
    },
    { field: 'name', headerName: 'Name', width: 300 },
    {
      field: 'surfaceArea',
      headerName: 'Surface Area',
      type: 'number',
      width: 150,
      valueFormatter: (params: GridValueFormatterParams<number>) => {
        return params.value.toLocaleString('en-US', { minimumFractionDigits: 2, maximumFractionDigits: 2 })
      },
    },
    {
      field: 'volume',
      headerName: 'Volume',
      type: 'number',
      width: 150,
      valueFormatter: (params: GridValueFormatterParams<number>) => {
        return params.value.toLocaleString('en-US', { minimumFractionDigits: 2, maximumFractionDigits: 2 })
      },
    },
    {
      field: 'components',
      headerName: 'Components',
      type: 'number',
      width: 100,
    },
    {
      field: 'handles',
      headerName: 'Handles',
      type: 'number',
      width: 100,
    }
  ];

  const rows = activeGroups.map((active, i) =>
  ({
    id: i,
    color: colors[i],
    name: active.name,
    surfaceArea: active.area,
    volume: active.volume,
    components: active.components,
    handles: active.handles,
  })
  );

  const [rowSelectionModel, setRowSelectionModel] = React.useState<GridRowSelectionModel>([]);
  useEffect(() => {
    setRowSelectionModel(
      activeGroups
        .map((e, i) => (e.on ? i : -1))
        .filter((e) => e !== -1)
    );
  }, [activeGroups]);

  const tableTest = (
    <div style={{ height: '100%', width: '100%' }}>
      <DataGrid
        rows={rows}
        columns={columns}
        initialState={{
          pagination: {
            paginationModel: { page: 0, pageSize: 10 },
          },
        }}
        pageSizeOptions={[10, 20]}
        checkboxSelection
        onRowSelectionModelChange={(newRowSelectionModel) => {
          setRowSelectionModel(newRowSelectionModel);
          const oldData = activeGroups;
          oldData.forEach((e, i) => {
            e.on = newRowSelectionModel.includes(i);
          });
          setActiveGroups([...oldData]);
        }}
        rowSelectionModel={rowSelectionModel}
      />
    </div>
  );

  const [leftTabValue, setLeftTabValue] = React.useState('1');
  const handleLeftTabChange = (event: React.SyntheticEvent, newValue: string) => {
    setLeftTabValue(newValue);
  };
  const [rightTabValue, setRightTabValue] = React.useState('1');
  const handleRightTabChange = (event: React.SyntheticEvent, newValue: string) => {
    setRightTabValue(newValue);
  };

  return !data ? (
    <Grid container style={{ width: "100%" }}>
      <Stack direction="row" spacing={2}>
        <Button variant="outlined" onClick={() => setIsOpen()}>
          Import Data
        </Button>
        {openExisting}
      </Stack>
    </Grid>
  ) : (
    <Stack direction="column" style={{ width: "100%" }}>
      <Stack direction="row" spacing={2} style={{ width: "100%", paddingBottom: 15 }}>
        <Button variant="outlined" onClick={() => setIsOpen()}>
          Import Data
        </Button>
        {openExisting}
        <Button onClick={saveHandler} variant="outlined">
          Save Geometry
        </Button>
        <Button onClick={() => setDoClusters((e) => !e)}>
          {doClusters ? "View Features" : "View Clusters"}
        </Button>
        <Button onClick={saveCanvas}>Download Picture</Button>
      </Stack>

      {/* FIXME: handle table overflow */}
      <Grid item container md={12} lg={12} spacing={3} style={{ overflow: scroll }}>
        <Grid item container md={6} lg={4}>
          <Paper
            style={{ width: "100%", height: "100%", boxSizing: "border-box", padding: 15 }}
            elevation={9}
          >
            <TabContext value={leftTabValue}>
              <Box sx={{ borderBottom: 1, borderColor: 'divider' }}>
                <TabList onChange={handleLeftTabChange} aria-label="lab API tabs example">
                  <Tab label="Slices" value="1" />
                  <Tab label="Groups" value="2" />
                  <Tab label="Settings" value="3" />
                  {/* <Tab label="Old Groups" value="4" /> */}
                </TabList>
              </Box>
              <TabPanel value="1">{sliceControl}</TabPanel>
              {/* <TabPanel value="4">{groupControl}</TabPanel> */}
              <TabPanel value="3">{viewControl}</TabPanel>
              <TabPanel value="2">{tableTest}</TabPanel>
            </TabContext>
          </Paper>
        </Grid>

        <Grid item container md={6} lg={8}>
          <Paper
            style={{ width: "100%", height: "100%", boxSizing: "border-box", padding: 15 }}
            elevation={9}
          >
            <TabContext value={rightTabValue}>
              <Box sx={{ borderBottom: 1, borderColor: 'divider' }}>
                <TabList onChange={handleRightTabChange} aria-label="lab API tabs example">
                  <Tab label="View" value="1" />
                  <Tab label="Groups" value="2" />
                </TabList>
              </Box>
              <TabPanel value="1">{renderArea}</TabPanel>
              <TabPanel value="2">{tableTest}</TabPanel>
            </TabContext>
          </Paper>
        </Grid>
      </Grid>
    </Stack>
  );
};
