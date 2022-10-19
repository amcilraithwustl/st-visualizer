//Every three points is a triangle rendered right hand rule
import * as THREE from "three";
import * as React from "react";
import { useLayoutEffect, useRef } from "react";

export function GeometryCustom({
  color,
  points,
  opacity,
  scale,
  translate,
}: // cap,
// join,
{
  color: JSX.IntrinsicElements["meshBasicMaterial"]["color"];
  opacity?: JSX.IntrinsicElements["meshBasicMaterial"]["opacity"];
  points: THREE.Vector3[];
  scale: number;
  translate: readonly [number, number, number];
}) {
  //create a blue LineBasicMaterial

  // create a simple square shape. We duplicate the top left and bottom right
  // vertices because each vertex needs to appear once per triangle.

  const ref = useRef<THREE.Mesh | null>(null);
  useLayoutEffect(() => {
    const temp = [...points, ...points.map((i) => i).reverse()]; //To ensure everything is double sided
    ref.current?.geometry
      .setFromPoints(temp)
      .translate(...translate)
      .scale(scale, scale, scale);
  }, [points, scale, translate]);
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
