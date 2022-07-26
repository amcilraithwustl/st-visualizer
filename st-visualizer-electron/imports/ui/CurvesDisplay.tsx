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

  curvesFinalData,
}: {
  center: THREE.Vector3;

  curvesFinalData: {
    group: number;
    pts: THREE.Vector3[];
  }[];
}): JSX.Element => {
  const translate = center
    ? ([-center.x, -center.y, -center.z] as const)
    : ([0, 0, 0] as const);

  return (
    <>
      {curvesFinalData.flatMap((ctr) => {
        return (
          <Line
            key={ctr.group}
            points={ctr.pts}
            color={colors[ctr.group] || defaultColor}
            translate={translate}
            scale={shrink}
          />
        );
      })}
    </>
  );
};
