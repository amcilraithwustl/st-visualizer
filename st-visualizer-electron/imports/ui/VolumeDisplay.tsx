import { colors, defaultColor, shrink } from "../api/constants";
import * as THREE from "three";
import * as React from "react";
import { GeometryCustom } from "./threejsComponents/GeometryCustom";
import { useMemo } from "react";
export const VolumeDisplay = ({
  center,
  volumes,
}: {
  center: THREE.Vector3;
  volumes: {
    group: number;
    pts: THREE.Vector3[];
  }[];
}): JSX.Element => {
  const v = useMemo(
    () =>
      volumes.map((ctr) => {
        return (
          <GeometryCustom
            key={ctr.group}
            points={ctr.pts}
            color={colors[ctr.group] || defaultColor}
            translate={
              center
                ? ([-center.x, -center.y, -center.z] as const)
                : ([0, 0, 0] as const)
            }
            scale={shrink}
            opacity={0.5}
          />
        );
      }),
    [center, volumes]
  );
  return <>{v}</>;
};
