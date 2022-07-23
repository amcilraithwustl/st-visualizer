import {
  colors,
  datatype,
  defaultColor,
  shrink,
  pointToVector,
} from "../api/constants";
import * as THREE from "three";
import { Line } from "./threejsComponents/Line";
import * as React from "react";

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
  return (
    <>
      {finalContours.flatMap((ctr, i) => {
        return (
          <Line
            key={i}
            points={ctr}
            color={colors[i] || defaultColor}
            translate={translate}
            scale={shrink}
          />
        );
      })}
    </>
  );
};
