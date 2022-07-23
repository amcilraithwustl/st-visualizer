import * as THREE from "three";
import * as React from "react";
import { useLayoutEffect, useRef } from "react";
import "../../api/threejsHeadeers";

export function Line({
  points,
  color,
  scale,
  translate,
}: // cap,
// join,
{
  points: THREE.Vector3[];
  color: JSX.IntrinsicElements["lineBasicMaterial"]["color"];
  scale: number;
  translate: readonly [number, number, number];
}) {
  //create a blue LineBasicMaterial

  // create a simple square shape. We duplicate the top left and bottom right
  // vertices because each vertex needs to appear once per triangle.

  const ref = useRef<THREE.LineSegments | null>(null);
  useLayoutEffect(() => {
    ref.current?.geometry
      .setFromPoints(points)
      .translate(...translate)
      .scale(scale, scale, scale);
  }, [points]);
  return (
    <lineSegments ref={ref}>
      <bufferGeometry />
      <lineBasicMaterial color={color} />
    </lineSegments>
  );
}
