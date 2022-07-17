import { extend, ReactThreeFiber } from "@react-three/fiber";
import * as THREE from "three";

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
