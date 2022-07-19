//Every three points is a triangle rendered right hand rule
import * as THREE from "three";
import * as React from "react";
import { useLayoutEffect, useRef } from "react";

export function GeometryCustom({
  color,
  points,
  opacity,
}: // cap,
// join,
{
  color: JSX.IntrinsicElements["meshBasicMaterial"]["color"];
  opacity?: JSX.IntrinsicElements["meshBasicMaterial"]["opacity"];
  points: THREE.Vector3[];
}) {
  //create a blue LineBasicMaterial

  // create a simple square shape. We duplicate the top left and bottom right
  // vertices because each vertex needs to appear once per triangle.

  const ref = useRef<THREE.Mesh>();
  useLayoutEffect(() => {
    ref.current.geometry.setFromPoints(points);
  }, [points]);
  return (
    <mesh ref={ref}>
      <bufferGeometry />
      <meshBasicMaterial
        color={color}
        opacity={opacity}
        transparent={opacity !== undefined}
      />
    </mesh>
  );
}
