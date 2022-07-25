import {
  colors,
  datatype,
  defaultColor,
  shrink,
  pointToVector,
  active,
} from "../api/constants";
import * as THREE from "three";
import * as React from "react";
import { GeometryCustom } from "./threejsComponents/GeometryCustom";

export const AreaDisplay = ({
  center,
  data,
  activeGroups,
  activeSlices,
}: {
  center: THREE.Vector3;
  data: datatype;
  activeGroups: active[];
  activeSlices: active[];
  nVals: number;
}): JSX.Element => {
  const translate = center
    ? ([-center.x, -center.y, -center.z] as const)
    : ([0, 0, 0] as const);
  const sortedFinalContours = React.useMemo(() => {
    const ctrs = data.tris2Dvals
      .slice(1, -1)
      .filter((_, i) => activeSlices[i]?.on)
      .map((slice) => ({
        points: slice[0],
        tris: slice[1],
        vals: slice[2],
      }));

    const sortedFinalContours = [...new Array(data.nat)].map(
      () => [] as THREE.Vector3[]
    );

    console.log("SORTED", sortedFinalContours);
    ctrs.flatMap((slice) =>
      slice.tris
        .map((indices, indexIndex) => ({
          triangle: indices.map((i) => pointToVector(slice.points[i])),
          val: slice.vals[indexIndex],
        }))
        .filter((item) => activeGroups[item.val].on)
        .forEach((triangle) =>
          sortedFinalContours[triangle.val].push(...triangle.triangle)
        )
    );
    return sortedFinalContours;
  }, [activeGroups, activeSlices, data.nat, data.tris2Dvals]);
  return (
    <>
      {sortedFinalContours.flatMap((ctr, i) => {
        return (
          <GeometryCustom
            key={i}
            points={ctr}
            color={colors[i] || defaultColor}
            translate={translate}
            scale={shrink}
            opacity={0.5}
          />
        );
      })}
    </>
  );
};
