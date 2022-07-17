import * as React from "react";
import { useRef, useState } from "react";
import * as THREE from "three";
import { useFrame } from "@react-three/fiber";

function Box(props: JSX.IntrinsicElements["mesh"]) {
  const ref = useRef<THREE.Mesh>(undefined);
  const ref2 = useRef<THREE.BoxGeometry>(undefined);
  const [hovered, hover] = useState(false);
  const [clicked, click] = useState(false);
  useFrame((state, delta) => (ref.current.rotation.x += 0.01));

  const segs = 1 * 2;
  console.log("totalItems", segs * segs * 6);
  return (
    <mesh
      {...props}
      ref={ref}
      scale={clicked ? 1.5 : 1}
      onClick={(event) => click(!clicked)}
      onPointerOver={(event) => hover(true)}
      onPointerOut={(event) => hover(false)}
    >
      <boxGeometry
        ref={ref2}
        attach={"geometry"}
        args={[8, 8, 8, segs, segs, segs]}
      />

      <meshStandardMaterial
        wireframe={true}
        color={hovered ? "hotpink" : "orange"}
      />
    </mesh>
  );
}
