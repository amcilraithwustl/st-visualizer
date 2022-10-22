//Every three points is a triangle rendered right hand rule
import * as THREE from "three";
import * as React from "react";
import { useLayoutEffect, useRef } from "react";
import { Face } from "@mui/icons-material";
import { ThreeElements } from "@react-three/fiber";

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
  const g = React.useMemo(() => {
    const a = new THREE.BufferGeometry();
    a.setFromPoints(points)
      .translate(...translate)
      .scale(scale, scale, scale)
      .computeVertexNormals();
    return a;
  }, [points, scale, translate]);

  return (
    <mesh>
      <bufferGeometry attributes={g.attributes} />
      <meshStandardMaterial
        color={color}
        opacity={opacity}
        transparent={opacity !== undefined}
        side={THREE.DoubleSide}
      />
    </mesh>
  );
}
