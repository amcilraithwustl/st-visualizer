import { defaultColor, shrink } from "../api/constants";
import * as THREE from "three";
import * as React from "react";
import { GeometryCustom } from "./threejsComponents/GeometryCustom";
import { Color } from "@react-three/fiber";

export const AreaDisplay = ({
  center,
  areaDisplayData,
  colors,
}: {
  center: THREE.Vector3;
  areaDisplayData: {
    group: number;
    pts: THREE.Vector3[];
  }[];
  colors: Color[];
}): JSX.Element => {
  const translate = React.useMemo(
    () =>
      center
        ? ([-center.x, -center.y, -center.z] as const)
        : ([0, 0, 0] as const),
    [center]
  );
  const v = React.useMemo(
    () =>
      areaDisplayData.flatMap((ctr) => {
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
      }),
    [areaDisplayData, colors, translate]
  );
  return <>{v}</>;
};
