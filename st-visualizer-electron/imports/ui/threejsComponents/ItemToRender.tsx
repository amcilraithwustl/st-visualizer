import * as React from "react";
import { useLayoutEffect, useRef, useState } from "react";
import * as THREE from "three";
import { Canvas, useFrame } from "@react-three/fiber";
import "../../api/threejsHeadeers";
import {
  CubeCamera,
  OrbitControls,
  PerspectiveCamera,
} from "@react-three/drei";

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

function Line({ points }: { points: THREE.Vector3[] }) {
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
      <lineBasicMaterial color={0x0000ff} />
    </line_>
  );
}

export const ItemToRender = () => (
  <Canvas style={{ height: 500, width: 500 }}>
    <ambientLight />
    <gridHelper args={[100, 10]} />
    <OrbitControls makeDefault enableDamping={false} />

    <pointLight position={[10, 10, 10]} />
    <Line
      points={[
        new THREE.Vector3(0, 1, 0),
        new THREE.Vector3(0, 0, 0),
        new THREE.Vector3(1, 0, 0),
        new THREE.Vector3(2, 1, 0),
      ]}
    />
  </Canvas>
);
