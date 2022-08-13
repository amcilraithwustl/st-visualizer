import React, { useEffect, useState } from "react";
import { Stack, Button, Grid, IconButton, Tooltip } from "@mui/material";
import { ArrowLeft, ArrowRight } from "@mui/icons-material";
export const AlignmentPage = (): JSX.Element => {
  const [currentImages, setCurrentImages] = useState<
    { file: File; alignments: { x: number; y: number; rotZ: number } }[]
  >([]);
  const [selectedImg, setSelectedImg] = useState<number | null>(null);
  const [imgUrls, setImgUrls] = useState<string[]>([]);
  useEffect(() => {
    const func = async () => {
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
            <Tooltip key={img} title={currentImages[i].file.name}>
              <Grid
                item
                sx={{ width: 100 }}
                onClick={() => i && setSelectedImg(i)}
              >
                <img
                  alt={currentImages[i].file.name}
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
  );
  return (
    <Stack style={{ padding: 20 }}>
      <Button variant="contained" component="label">
        Alignment Images
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
                .map((f) => ({ file: f, alignments: { x: 0, y: 0, rotZ: 0 } }))
            );
          }}
        />
      </Button>
      {imageOrdering}
    </Stack>
  );
};
