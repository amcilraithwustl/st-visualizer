import {
  colors,
  datatype,
  defaultColor,
  shrink,
  pointToVector,
} from "../api/constants";
import * as THREE from "three";
import * as React from "react";
import { GeometryCustom } from "./threejsComponents/GeometryCustom";
import { useMemo } from "react";
export const VolumeDisplay = ({
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

  function createPolygon(poly: THREE.Vector3[]) {
    //Assuming the polygon is a star
    const centerPoint = poly
      .reduce((a, b) => a.add(b), new THREE.Vector3(0, 0, 0))
      .divideScalar(poly.length);

    return poly.flatMap((e, i) => {
      return [poly[i], poly[(i + 1) % poly.length], centerPoint];
    });
  }

  const volumes = useMemo(
    () =>
      data.ctrs3Dvals
        .map((val) => ({
          points: val[0].map((p) => pointToVector(p)),
          rings: val[1],
        }))
        .map((val) =>
          val.rings.flatMap((indices) =>
            createPolygon(indices.map((index) => val.points[index]))
          )
        ),
    [data.ctrs3Dvals]
  );
  console.log("VOLUMES", volumes);

  return (
    <>
      {volumes.flatMap((ctr, i) => {
        console.log("COLOR", i, colors[i]);
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
