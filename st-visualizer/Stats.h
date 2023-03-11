#pragma once
#include "UtilityFunctions.h"

Eigen::Vector3f cross(const Eigen::Vector3f& A, const Eigen::Vector3f& B, const Eigen::Vector3f& C) {
	return(B - A).cross(C - A);
}

//The positive norm defines the direction that is "positive area"
float triArea(const Eigen::Vector3f& A, const Eigen::Vector3f& B, const Eigen::Vector3f& C, const Eigen::Vector3f& positiveNorm) {
	//One half the cross product is the area of a triangle defined by three points
	auto crossProduct = cross(A, B, C);
	bool isPositive = true;//TODO: Figure out how to make this work //crossProduct.normalized() == positiveNorm;
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