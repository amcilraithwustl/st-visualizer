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
  areaDisplayData,
}: {
  center: THREE.Vector3;
  areaDisplayData: {
    group: number;
    pts: THREE.Vector3[];
  }[];
}): JSX.Element => {
  const translate = center
    ? ([-center.x, -center.y, -center.z] as const)
    : ([0, 0, 0] as const);

  return (
    <>
      {areaDisplayData.flatMap((ctr) => {
        return (
          <GeometryCustom
            key={ctr.group}
            points={ctr.pts}
            color={colors[ctr.group] || defaultColor}
            translate={translate}
            scale={shrink}
            opacity={0.5}
          />
        );
      })}
    </>
  );
};
