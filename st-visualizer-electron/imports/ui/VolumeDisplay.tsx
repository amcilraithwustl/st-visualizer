import { defaultColor, shrink } from "../api/constants";
import * as THREE from "three";
import * as React from "react";
import { GeometryCustom } from "./threejsComponents/GeometryCustom";
import { useMemo } from "react";
import { Color } from "@react-three/fiber";

export const VolumeDisplay = ({
  center,
  volumes,
  colors,
  opacity,
}: {
  center: THREE.Vector3;
  volumes: {
    group: number;
    pts: THREE.Vector3[];
  }[];
  colors: Color[];
  opacity: number;
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
      volumes.map((ctr) => {
        return (
          <GeometryCustom
            key={ctr.group}
            points={ctr.pts}
            color={colors[ctr.group] || defaultColor}
            translate={translate}
            scale={shrink}
            opacity={opacity}
          />
        );
      }),
    [colors, opacity, translate, volumes]
  );
  return <>{v}</>;
};
