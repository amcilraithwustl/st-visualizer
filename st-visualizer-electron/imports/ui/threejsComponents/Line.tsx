import * as THREE from "three";
import * as React from "react";
import { useLayoutEffect, useRef } from "react";

export function Line({
  points,
  color,
}: // cap,
// join,
{
  points: THREE.Vector3[];
  color: JSX.IntrinsicElements["lineBasicMaterial"]["color"];
}) {
  //create a blue LineBasicMaterial

  // create a simple square shape. We duplicate the top left and bottom right
  // vertices because each vertex needs to appear once per triangle.

  const ref = useRef<THREE.Line>();
  useLayoutEffect(() => {
    ref.current.geometry.setFromPoints(points);
  }, [points]);
  return (
    <line_ ref={ref}>
      <bufferGeometry />
      <lineBasicMaterial color={color} />
    </line_>
  );
}
