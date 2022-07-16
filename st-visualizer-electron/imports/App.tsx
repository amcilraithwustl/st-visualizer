import "react";
import * as React from "react";
import * as THREE from "three";
import { useEffect, useLayoutEffect, useRef, useState } from "react";
import { Canvas, useFrame } from "@react-three/fiber";
import { ReactThreeFiber, extend } from "@react-three/fiber";

//https://www.npmjs.com/package/@react-three/drei
//Fix typescript problem: https://github.com/pmndrs/react-three-fiber/discussions/1387

extend({ Line_: THREE.Line });

declare global {
  namespace JSX {
    interface IntrinsicElements {
      line_: ReactThreeFiber.Object3DNode<THREE.Line, typeof THREE.Line>;
    }
  }
}

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

function Line(props: JSX.IntrinsicElements["line_"]) {
  //create a blue LineBasicMaterial

  // create a simple square shape. We duplicate the top left and bottom right
  // vertices because each vertex needs to appear once per triangle.

  const ref = useRef<THREE.Line>();
  useLayoutEffect(() => {
    const points = [];
    points.push(new THREE.Vector3(0, 10, 0));
    points.push(new THREE.Vector3(0, 0, 0));
    points.push(new THREE.Vector3(10, 0, 0));
    ref.current.geometry.setFromPoints(points);
  }, []);
  return (
    <line_ {...props} ref={ref}>
      <bufferGeometry />
      <lineBasicMaterial color={0x0000ff} />
    </line_>
  );
}

const ItemToRender = () => (
  <Canvas style={{ height: 500, width: 500 }}>
    <ambientLight />
    <pointLight position={[10, 10, 10]} />
    <Line position={[0, 0, 0]} />
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
