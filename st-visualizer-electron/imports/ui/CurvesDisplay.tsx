import {
  active,
  colors,
  datatype,
  defaultColor,
  shrink,
  pointToVector,
} from "../api/constants";
import * as THREE from "three";
import { Points } from "./threejsComponents/Points";
import { Line } from "./threejsComponents/Line";
import * as React from "react";
import { Vector3 } from "three";
import { ThreeElements } from "@react-three/fiber";

export const CurvesDisplay = ({
  center,
  data,
}: {
  center: THREE.Vector3;

  data: datatype;
}): JSX.Element => {
  const translate = center
    ? ([-center.x, -center.y, -center.z] as const)
    : ([0, 0, 0] as const);

  const testCtrs = data.ctrs2Dvals.flatMap((slice) =>
    slice.flatMap((ctr) => ctr[0].flatMap((pt) => pointToVector(pt)))
  );

  const ctrs = data.ctrs2Dvals.map((slice) =>
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
  const finalContours = ctrs.reduce(
    (acc, slice) => {
      slice.forEach((ctr) => acc[ctr.val].push(...ctr.segments));
      return acc;
    },
    [...new Array(ctrs[1].length)].map(() => [] as THREE.Vector3[])
  );
  console.log("CONTOURS", ctrs);
  return (
    <>
      {finalContours.flatMap((ctr, i) => {
        console.log("CONTOUR", ctr);
        return (
          <Line
            key={i}
            points={ctr}
            color={colors[i]}
            translate={translate}
            scale={shrink}
          />
        );
      })}
    </>
  );
};
