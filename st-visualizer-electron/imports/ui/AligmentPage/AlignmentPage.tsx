import React, { useEffect, useState } from "react";
import { Stack, Button } from "@mui/material";
export const AlignmentPage = (): JSX.Element => {
  const [currentImages, setCurrentImages] = useState<File[]>([]);
  const [imgUrls, setImgUrls] = useState<string[]>([]);
  useEffect(() => {
    const func = async () => {
      const urls = await Promise.all(
        currentImages.map(
          (file) =>
            new Promise<string>((res) => {
              const reader = new FileReader();
              reader.readAsDataURL(file);
              reader.onloadend = function () {
                res(reader.result as string);
              };
            })
        )
      );
      console.log("URLS", urls);
      setImgUrls(urls);
    };
    func();
  }, [currentImages]);
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
              [...new Array(files.length)].map((_, i) => files[i])
            );
          }}
        />
      </Button>
      {imgUrls?.map((img) => {
        return <img key={img} alt={img} src={img} />;
      })}
    </Stack>
  );
};
