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
import { ThreeElements } from "@react-three/fiber";
import { Triangle } from "three";
import { GeometryCustom } from "./threejsComponents/GeometryCustom";

export const AreaDisplay = ({
  center,
  data,
}: {
  center: THREE.Vector3;
  data: datatype;
}): JSX.Element => {
  const translate = center
    ? ([-center.x, -center.y, -center.z] as const)
    : ([0, 0, 0] as const);

  const ctrs = data.tris2Dvals.map((slice) => ({
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
      .forEach((triangle) =>
        sortedFinalContours[triangle.val].push(...triangle.triangle)
      )
  );
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
