import * as THREE from "three";
import * as React from "react";
import { useLayoutEffect, useRef } from "react";
import "../../api/threejsHeadeers";

export function Points({
  points,
  color,
  scale,
  translate,
}: {
  points: THREE.Vector3[];
  color: JSX.IntrinsicElements["lineBasicMaterial"]["color"];
  scale: number;
  translate: readonly [number, number, number];
}) {
  //create a blue LineBasicMaterial

  // create a simple square shape. We duplicate the top left and bottom right
  // vertices because each vertex needs to appear once per triangle.

  const ref2 = useRef<THREE.Points | null>(null);
  useLayoutEffect(() => {
    ref2.current?.geometry
      .setFromPoints(points)
      .translate(...translate)
      .scale(scale, scale, scale);
  }, [points, scale, translate]);
  return (
    <points ref={ref2}>
      <bufferGeometry />
      <pointsMaterial color={color} size={0.1} sizeAttenuation={true} />
    </points>
  );
}
