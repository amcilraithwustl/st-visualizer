#pragma once
#include "UtilityFunctions.h"

float triArea(Eigen::Vector3f A, Eigen::Vector3f B, Eigen::Vector3f C) {
	//One half the cross product is the area of a triangle defined by three points
	auto crossProduct = (B - A).cross(C - A);
	return (0.5f) * crossProduct.norm();
}