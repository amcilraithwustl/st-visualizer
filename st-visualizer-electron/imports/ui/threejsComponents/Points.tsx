import * as THREE from "three";
import * as React from "react";
import { useLayoutEffect, useRef } from "react";

export function Points({
  points,
  color,
}: {
  points: THREE.Vector3[];
  color: JSX.IntrinsicElements["lineBasicMaterial"]["color"];
}) {
  //create a blue LineBasicMaterial

  // create a simple square shape. We duplicate the top left and bottom right
  // vertices because each vertex needs to appear once per triangle.

  const ref2 = useRef<THREE.Points>();
  useLayoutEffect(() => {
    ref2.current.geometry.setFromPoints(points);
  }, [points]);
  return (
    <points ref={ref2}>
      <bufferGeometry />
      <pointsMaterial color={color} size={0.1} />
    </points>
  );
}
