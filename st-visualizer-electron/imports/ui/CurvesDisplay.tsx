import { defaultColor, shrink } from "../api/constants";
import * as THREE from "three";
import { Line } from "./threejsComponents/Line";
import * as React from "react";
import { Color } from "@react-three/fiber";

export const CurvesDisplay = ({
  center,
  colors,
  curvesFinalData,
}: {
  center: THREE.Vector3;
  colors: Color[];

  curvesFinalData: {
    group: number;
    pts: THREE.Vector3[];
  }[];
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
      curvesFinalData.flatMap((ctr) => {
        return (
          <Line
            key={ctr.group}
            points={ctr.pts}
            color={colors[ctr.group] || defaultColor}
            translate={translate}
            scale={shrink}
          />
        );
      }),
    [colors, curvesFinalData, translate]
  );
  return <>{v}</>;
};
