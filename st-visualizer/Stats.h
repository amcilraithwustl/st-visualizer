#pragma once
#include "UtilityFunctions.h"

Eigen::Vector3f cross(const Eigen::Vector3f& A, const Eigen::Vector3f& B, const Eigen::Vector3f& C) {
	return(B - A).cross(C - A);
}

//The positive norm defines the direction that is "positive area"
float triArea(const Eigen::Vector3f& A, const Eigen::Vector3f& B, const Eigen::Vector3f& C, const Eigen::Vector3f& positiveNorm) {
	//One half the cross product is the area of a triangle defined by three points
	auto crossProduct = cross(A, B, C);
	//TODO: Figure out how to make this work Not sure these are all coplanar
	// bool isPositive = crossProduct.normalized() == positiveNorm;
	bool isPositive = true;
	return (0.5f) * crossProduct.norm() * (isPositive ? 1 : -1);
}

std::vector<float> getSurfaceAreas(std::vector<std::pair<std::vector<Eigen::Vector3f>, std::vector<std::vector<int>>>> info) {
	std::vector<float> surfaceAreas = {};
	for (const auto& elem : info) {
		const auto& pts = elem.first;
		const auto& faces = elem.second;
		float surfaceArea = 0;
		for (const auto& face : faces) {
			auto tempSurface = 0;
			Eigen::Vector3f positiveNorm = cross(pts[face[0]], pts[face[1]], pts[face[2]]).normalized();
			for (int i = 1; i < face.size() - 1; i++) {
				tempSurface += triArea(pts[face[0]], pts[face[i]], pts[face[i + 1]], positiveNorm);
			}
			surfaceArea += abs(tempSurface);
		}
		surfaceAreas.push_back(surfaceArea);
	}
	return surfaceAreas;
}


// Volume of a tet w/ vertex at origin is 1/6 the determinant of a 3x3 matrix formed by the remaining vertices
// This volume is signed based on the orientation of the face. This allows us to calculate the total volume of a mesh.
// https://n-e-r-v-o-u-s.com/blog/?p=4415#:~:text=The%20idea%20behind%20calculating%20the,%2C0)%20to%20the%20triangle.

float getTetVolume(const Eigen::Vector3f& A, const Eigen::Vector3f& B, const Eigen::Vector3f& C) {
	return A.cross(B).dot(C) / 6;
}

std::vector<double> getVolumes(std::vector<std::pair<std::vector<Eigen::Vector3f>, std::vector<std::vector<int>>>> info) {
	std::vector<double> volumes = {};
	for (const auto& elem : info) {
		const auto& pts = elem.first;
		const auto& faces = elem.second;
		double volume = 0;
		for (const auto& face : faces) {
			double tempSurface = 0;
			for (int i = 1; i < face.size() - 1; i++) {
				tempSurface += getTetVolume(pts[face[0]], pts[face[i]], pts[face[i + 1]]);
			}
			volume += (tempSurface);
		}
		volumes.push_back(abs(volume));
	}
	return volumes;
}