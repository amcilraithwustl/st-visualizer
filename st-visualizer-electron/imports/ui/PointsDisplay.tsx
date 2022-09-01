import { defaultColor, shrink } from "../api/constants";
import * as THREE from "three";
import { Points } from "./threejsComponents/Points";
import * as React from "react";
import { Color } from "@react-three/fiber";

export const PointsDisplay = ({
  center,
  groups,
  colors,
}: {
  center: THREE.Vector3;
  groups: {
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
      Object.values(groups).map((g) => {
        return (
          <Points
            key={g.group}
            points={g.pts}
            translate={translate}
            scale={shrink}
            color={colors[g.group] || defaultColor}
          />
        );
      }),
    [colors, groups, translate]
  );
  return <>{v}</>;
};
