import React, { useEffect, useMemo, useRef, useState } from "react";
import {
  Stack,
  Button,
  Grid,
  IconButton,
  Tooltip,
  Slider,
  Input,
  Typography,
} from "@mui/material";
import { ArrowLeft, ArrowRight } from "@mui/icons-material";
import {
  importStateType,
  colTypes,
  importPts,
  datatype,
} from "../../api/constants";
import _ from "lodash";
export type transformType = {
  file: File;
  alignments: {
    x: number;
    y: number;
    rotZ: number;
  };
};

const getImg = (src: string) =>
  new Promise<HTMLImageElement>((res) => {
    const img = new Image();
    img.src = src;
    img.onload = () => {
      res(img);
    };
  });

export const calcTransforms = (
  currentImages: transformType[],
  scale: number
) => {
  const startRadius = 1000;
  const startPointLocations = [
    { x: 0, y: 0 },
    { x: startRadius, y: startRadius },
    { x: -startRadius, y: -startRadius },
    { x: -startRadius, y: startRadius },
    { x: startRadius, y: -startRadius },
  ];
  const rotate = ({ x, y, angle }: { x: number; y: number; angle: number }) => {
    const radAngle = (angle * Math.PI) / 180;
    return {
      x: x * Math.cos(radAngle) + y * Math.sin(radAngle),
      y: y * Math.cos(radAngle) - x * Math.sin(radAngle),
    };
  };

  const transforms = currentImages.reduceRight((arr, { alignments }) => {
    const appendedArray = [startPointLocations, ...arr];
    //Rotate then transform
    const transformed = appendedArray.map((prev) =>
      prev
        .map((p) =>
          rotate({
            x: p.x,
            y: p.y,
            angle: alignments.rotZ,
          })
        )
        .map((rotated) => ({
          x: rotated.x + alignments.x,
          y: rotated.y + alignments.y,
        }))
    );
    return transformed;
  }, [] as { x: number; y: number }[][]);
  return transforms.map((slice) =>
    slice.map((pt) => ({ x: pt.x * scale, y: pt.y * scale }))
  );
};

export const AlignmentPage = ({
  importState,
  setImportState,
  currentImages,
  setCurrentImages,
  setData,
}: {
  importState: importStateType;
  setImportState: React.Dispatch<React.SetStateAction<importStateType>>;
  setCurrentImages: React.Dispatch<React.SetStateAction<transformType[]>>;
  setData: React.Dispatch<React.SetStateAction<datatype>>;
  currentImages: transformType[];
}): JSX.Element => {
  const sliceNames = _.compact(
    _.uniq(
      importState.tsvData.map((row, i) =>
        i > 0 ? row[importState[colTypes.slice]] : undefined
      )
    )
  );
  const [selectedImg, setSelectedImg] = useState<number | null>(null);
  const [imgUrls, setImgUrls] = useState<string[]>([]);
  const [scale, setScale] = useState<number>(100);
  const transforms = calcTransforms(currentImages, scale);
  const slicesRow = importState.tsvData.map(
    (row) => row[importState[colTypes.slice]]
  );
  //Slice to remove the title and the compact to remove undefined
  const numSlices = _.compact(_.uniq(slicesRow.slice(1))).length;
  console.log("CURRENT Transforms", transforms, sliceNames);
  const xCol = importState[colTypes.column];
  const yCol = importState[colTypes.row];
  const currentVals = importState.tsvData
    .map((row, i) =>
      i === 0
        ? undefined
        : {
            x: parseFloat(row[xCol]),
            y: parseFloat(row[yCol]),
            slice: importState.tsvData[importState[colTypes.slice]],
          }
    )
    .filter((v) => !!v);
  console.log(currentVals);
  useEffect(() => {
    const func = async () => {
      console.log("RUNNING");
      const urls = await Promise.all(
        currentImages.map(
          (file) =>
            new Promise<string>((res) => {
              const reader = new FileReader();
              reader.readAsDataURL(file.file);
              reader.onloadend = function () {
                res(reader.result as string);
              };
            })
        )
      );
      setImgUrls(urls);
    };
    func();
  }, [currentImages]);
  const imageOrdering = (
    <Stack>
      <Stack direction="row">
        {imgUrls?.map((img, i) => {
          return (
            <Stack
              direction="row"
              key={img}
              style={{
                borderBottom:
                  selectedImg !== null &&
                  (i === selectedImg || i === selectedImg - 1)
                    ? "solid 1px black"
                    : undefined,
              }}
            >
              <Grid item>
                <IconButton
                  onClick={() => {
                    if (i === 0) return;
                    const temp = [...currentImages];
                    [temp[i], temp[i - 1]] = [temp[i - 1], temp[i]];
                    setCurrentImages(temp);
                  }}
                >
                  <ArrowLeft />
                </IconButton>
              </Grid>
              <Tooltip key={img} title={currentImages[i]?.file.name}>
                <Grid
                  item
                  sx={{ width: 100 }}
                  onClick={() => i && setSelectedImg(i)}
                >
                  <img
                    alt={currentImages[i]?.file.name}
                    src={img}
                    style={{ maxWidth: "100%" }}
                  />
                </Grid>
              </Tooltip>
              <Grid item>
                <IconButton
                  onClick={() => {
                    if (i === currentImages.length - 1) return;
                    const temp = [...currentImages];
                    [temp[i], temp[i + 1]] = [temp[i + 1], temp[i]];
                    setCurrentImages(temp);
                  }}
                >
                  <ArrowRight />
                </IconButton>
              </Grid>
            </Stack>
          );
        })}
      </Stack>
      <Stack direction="row">
        {importState.sliceOrder?.map((nameIndex, i) => {
          return (
            <Stack direction="row" key={nameIndex}>
              <Grid item>
                <IconButton
                  onClick={() => {
                    if (i === 0) return;
                    const temp = [...importState.sliceOrder];
                    [temp[i], temp[i - 1]] = [temp[i - 1], temp[i]];
                    setImportState((s) => ({ ...s, sliceOrder: temp }));
                  }}
                >
                  <ArrowLeft />
                </IconButton>
              </Grid>
              <Grid item sx={{ width: 100 }}>
                {sliceNames[nameIndex]}
              </Grid>
              <Grid item>
                <IconButton
                  onClick={() => {
                    if (i === currentImages.length - 1) return;
                    const temp = [...importState.sliceOrder];
                    [temp[i], temp[i + 1]] = [temp[i + 1], temp[i]];
                    setImportState((s) => ({ ...s, sliceOrder: temp }));
                  }}
                >
                  <ArrowRight />
                </IconButton>
              </Grid>
            </Stack>
          );
        })}
      </Stack>
    </Stack>
  );
  const [opacity, setOpacity] = useState(0.8);
  const alignments = selectedImg && currentImages[selectedImg]?.alignments;
  const handleXChange = (_: unknown, n: number) => {
    if (null === selectedImg) return;
    const newCurrImgs = [...currentImages];
    newCurrImgs[selectedImg].alignments.x = n;
    setCurrentImages(newCurrImgs);
  };
  const handleYChange = (_: unknown, n: number) => {
    if (null === selectedImg) return;
    const newCurrImgs = [...currentImages];
    newCurrImgs[selectedImg].alignments.y = n;
    setCurrentImages(newCurrImgs);
  };
  const handleRotZChange = (_: unknown, n: number) => {
    if (null === selectedImg) return;
    const newCurrImgs = [...currentImages];
    newCurrImgs[selectedImg].alignments.rotZ = n;
    setCurrentImages(newCurrImgs);
  };
  const mainImg = useRef<HTMLImageElement | null>(null);
  const [height, setHeight] = useState(0);
  const [width, setWidth] = useState(0);

  useEffect(() => {
    if (!selectedImg) return;
    getImg(imgUrls[selectedImg]).then((img) => {
      setHeight(img.naturalHeight);
      setWidth(img.naturalWidth);
    });
  }, [imgUrls, selectedImg]);
  const naturalHeight = height;
  const naturalWidth = width;
  console.log(naturalWidth, naturalHeight);

  const displayArea = alignments &&
    selectedImg &&
    selectedImg < currentImages.length && (
      <Grid
        style={{ width: "100%", position: "relative" }}
        direction="row"
        spacing={2}
      >
        {/* Image Display */}
        <Grid item>
          <div
            style={{
              width: "500px",
              height: "500px",
              position: "relative",
              pointerEvents: "none",
            }}
          >
            <img
              src={imgUrls[selectedImg - 1]}
              style={{
                width: "100%",
                height: "100%",
                objectFit: "contain",
                position: "relative",
              }}
            />
            <img
              ref={mainImg}
              src={imgUrls[selectedImg]}
              style={{
                position: "absolute",
                width: "100%",
                height: "100%",
                objectFit: "contain",
                left: 0,
                top: 0,
                opacity,
                transform:
                  "translateX(" +
                  (alignments.x / naturalWidth) * 100 +
                  "%)translateY(" +
                  (alignments.y / naturalHeight) * 100 +
                  "%)rotate(" +
                  alignments.rotZ +
                  "deg)",
              }}
            />
          </div>
        </Grid>
        {/* Control Area */}
        <Grid item container sx={{ minWidth: 300 }} xs={12}>
          <Grid container item spacing={2} alignItems="center" xs={12}>
            <Typography>
              <em>Bottom</em>
            </Typography>
            <Grid item xs>
              <Slider
                color="secondary"
                value={opacity}
                step={0.01}
                min={0}
                max={1}
                onChange={(_, n) => typeof n === "number" && setOpacity(n)}
              />
            </Grid>
            <Typography>
              <em>Top</em>
            </Typography>
          </Grid>

          {/* X Position */}
          <Grid container item spacing={2} alignItems="center" xs={12}>
            <Typography>X: </Typography>

            <Grid item xs>
              <Slider
                min={-naturalWidth}
                max={naturalWidth}
                step={0.1}
                value={alignments.x}
                onChange={handleXChange}
              />
            </Grid>
            <Grid item>
              <Input
                size="small"
                inputProps={{
                  step: 0.01,
                  min: -naturalWidth,
                  max: naturalWidth,
                  type: "number",
                }}
                onChange={(e) =>
                  handleXChange(null, parseFloat(e.target.value))
                }
                value={alignments.x}
              />
            </Grid>
          </Grid>

          {/* Y Position */}
          <Grid container item spacing={2} alignItems="center" xs={12}>
            <Typography>Y: </Typography>

            <Grid item xs>
              <Slider
                min={-naturalHeight}
                max={naturalHeight}
                step={0.1}
                value={alignments.y}
                onChange={handleYChange}
              />
            </Grid>
            <Grid item>
              <Input
                size="small"
                inputProps={{
                  step: 0.01,
                  min: -naturalHeight,
                  max: naturalHeight,
                  type: "number",
                }}
                onChange={(e) =>
                  handleYChange(null, parseFloat(e.target.value))
                }
                value={alignments.y}
              />
            </Grid>
          </Grid>

          {/* Z Rotation */}
          <Grid container item spacing={2} alignItems="center" xs={12}>
            <Typography>Rotation: </Typography>

            <Grid item xs>
              <Slider
                min={0}
                max={360}
                step={0.1}
                value={alignments.rotZ}
                onChange={handleRotZChange}
              />
            </Grid>
            <Grid item>
              <Input
                size="small"
                inputProps={{
                  step: 0.01,
                  min: 0,
                  max: 360,
                  type: "number",
                }}
                onChange={(e) =>
                  handleRotZChange(null, parseFloat(e.target.value))
                }
                value={alignments.rotZ}
              />
            </Grid>
          </Grid>
        </Grid>
      </Grid>
    );
  const lengthsMatch = currentImages.length === numSlices;

  return (
    <Stack style={{ padding: 20 }}>
      <Tooltip
        title={
          lengthsMatch
            ? "Press to calculate volumes"
            : "Alignment mismatch. " + numSlices + " slices required"
        }
      >
        <Button
          color={lengthsMatch ? "secondary" : "error"}
          onClick={async () => {
            if (lengthsMatch) {
              console.log("BEGINNING");
              const path = await window.electronAPI.doCalculation({
                transforms: calcTransforms(currentImages, scale),
                importState,
              });
              if (!path) return;
              console.log("PATH", path);
              const result = await importPts(path);
              if (!result) return;
              console.log("RESULT", result);
              setData(result);
            } else {
              console.log("DING DING");
              const result = await importPts(
                "C:/Users/Aiden McIlraith/Documents/GitHub/st-visualizer/st-visualizer-electron/.webpack/main/output.json"
              );
              if (!result) return;
              console.log("RESULT", result);
              setData(result);
            }
          }}
        >
          Run Final Calculation{lengthsMatch ? "" : " (Not Ready)"}
        </Button>
      </Tooltip>
      <Button variant="contained" component="label">
        Alignment Images ({numSlices} needed)
        <input
          hidden
          multiple
          accept="image/*"
          type="file"
          onChange={(e) => {
            const files = e.target.files;

            if (!files) return;
            setCurrentImages(
              [...new Array(files.length)]
                .map((_, i) => files[i])
                .map((f) => ({
                  file: f,
                  alignments: { x: 0, y: 0, rotZ: 0 },
                }))
            );
            setImportState((s) => ({
              ...s,
              sliceOrder: [...new Array(numSlices)].map((_, i) => i),
            }));
          }}
        />
      </Button>
      {imageOrdering}
      <Grid container item spacing={2} alignItems="center" xs={12}>
        <Typography>Scale (pixels per micron): </Typography>
        <Grid item>
          <Input
            size="small"
            inputProps={{
              step: 0.01,
              min: 0,
              max: 360,
              type: "number",
            }}
            onChange={(e) => setScale(parseFloat(e.target.value))}
            value={scale}
          />
        </Grid>
      </Grid>
      {displayArea}
    </Stack>
  );
};
