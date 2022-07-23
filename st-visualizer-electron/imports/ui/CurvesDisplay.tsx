import {
  active,
  colors,
  datatype,
  defaultColor,
  shrink,
  pointToVector,
} from "../api/constants";
import * as THREE from "three";
import { Points } from "./threejsComponents/Points";
import { Line } from "./threejsComponents/Line";
import * as React from "react";
import { Vector3 } from "three";

export const CurvesDisplay = ({
  center,
  data,
}: {
  center: THREE.Vector3;

  data: datatype;
}): JSX.Element => {
  const translate = center
    ? ([-center.x, -center.y, -center.z] as const)
    : ([0, 0, 0] as const);

  const ctrs = data.ctrs2Dvals.flatMap((slice) =>
    slice.flatMap((ctr) => ctr[0].flatMap((pt) => pointToVector(pt)))
  );
  return (
    <>
      <Points
        points={ctrs}
        translate={translate}
        scale={shrink}
        color={"black"}
      />
    </>
  );
};
