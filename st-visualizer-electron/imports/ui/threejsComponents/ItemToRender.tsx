import * as React from "react";
import * as THREE from "three";
import { Canvas } from "@react-three/fiber";
import "../../api/threejsHeadeers";
import { GizmoHelper, GizmoViewport, OrbitControls } from "@react-three/drei";
import { Line } from "./Line";
import { Points } from "./Points";

export const ItemToRender = () => {
  return (
    <Canvas style={{ height: 500 }}>
      <ambientLight />
      <gridHelper args={[100, 10]} />
      <OrbitControls makeDefault enableDamping={false} />
      <GizmoHelper
        alignment="top-right" // widget alignment within scene
        margin={[80, 80]} // widget margins (X, Y)
      >
        <GizmoViewport
          axisColors={["red", "green", "blue"]}
          labelColor="black"
        />
      </GizmoHelper>
      <pointLight position={[10, 10, 10]} />
      <Line
        points={[
          new THREE.Vector3(0, 1, 0),
          new THREE.Vector3(0, 0, 0),
          new THREE.Vector3(1, 0, 0),
          new THREE.Vector3(2, 1, 0),
        ]}
        color={"red"}
      />
      <Points
        points={[
          new THREE.Vector3(0, 1, 1),
          new THREE.Vector3(0, 0, 1),
          new THREE.Vector3(1, 0, 1),
          new THREE.Vector3(2, 1, 1),
        ]}
        color={"green"}
      />
    </Canvas>
  );
};
