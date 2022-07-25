import {
  colors,
  datatype,
  defaultColor,
  shrink,
  pointToVector,
  active,
} from "../api/constants";
import * as THREE from "three";
import { Line } from "./threejsComponents/Line";
import * as React from "react";
import { useMemo } from "react";

export const CurvesDisplay = ({
  center,
  ctrs2Dvals,
  activeGroups,
  activeSlices,
  nVals,
}: {
  center: THREE.Vector3;
  ctrs2Dvals: datatype["ctrs2Dvals"];
  activeGroups: active[];
  activeSlices: active[];
  nVals: number;
}): JSX.Element => {
  console.log(activeGroups, activeSlices, ctrs2Dvals);
  const translate = center
    ? ([-center.x, -center.y, -center.z] as const)
    : ([0, 0, 0] as const);

  const finalContours = useMemo(() => {
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
        [...new Array(nVals)].map(() => [] as THREE.Vector3[])
      )
      .filter((_, i) => activeGroups[i]?.on);
  }, [activeGroups, activeSlices, ctrs2Dvals, nVals]);
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
