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
import { useMemo } from "react";
export const VolumeDisplay = ({
  center,
  activeGroups,
  volumes,
}: {
  center: THREE.Vector3;
  activeGroups: active[];
  volumes: {
    group: number;
    pts: THREE.Vector3[];
  }[];
}): JSX.Element => {
  const translate = useMemo(
    () =>
      center
        ? ([-center.x, -center.y, -center.z] as const)
        : ([0, 0, 0] as const),
    [center]
  );

  const v = useMemo(
    () =>
      volumes.map((ctr, i) => {
        if (!activeGroups[ctr.group].on) return undefined;
        return (
          <GeometryCustom
            key={i}
            points={ctr.pts}
            color={colors[ctr.group] || defaultColor}
            translate={translate}
            scale={shrink}
            opacity={0.5}
          />
        );
      }),
    [activeGroups, translate, volumes]
  );
  return <>{v}</>;
};
