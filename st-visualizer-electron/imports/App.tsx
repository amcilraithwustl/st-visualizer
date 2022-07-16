import "react";
import * as React from "react";
import * as THREE from "three";
import { useRef, useState } from "react";
import { Canvas, useFrame } from "@react-three/fiber";
//https://www.npmjs.com/package/@react-three/drei

function Box(props: JSX.IntrinsicElements["mesh"]) {
  const ref = useRef<THREE.Mesh>(undefined);
  const ref2 = useRef<THREE.BoxGeometry>(undefined);
  const [hovered, hover] = useState(false);
  const [clicked, click] = useState(false);
  useFrame((state, delta) => (ref.current.rotation.x += 0.01));

  const segs = 100 * 2;
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

const ItemToRender = () => (
  <Canvas style={{ height: 500, width: 500 }}>
    <ambientLight />
    <pointLight position={[10, 10, 10]} />
    <Box position={[0, 1, -10]} />
  </Canvas>
);

export const App = () => {
  return (
    <div>
      React/Webgl Example
      <div
        style={{ borderWidth: 3, borderColor: "black", borderStyle: "solid" }}
      >
        <ItemToRender />
      </div>
    </div>
  );
};
