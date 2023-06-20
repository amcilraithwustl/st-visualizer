#pragma once

#include "UtilityFunctions.h"

#include <Eigen/Dense>
#include <functional>
#include <vector>

colCoordMat listToMatrix(std::list<coord> source)
{
	// Row 0 is x, row 1 is y
	colCoordMat sourceMatrix(2, source.size());
	size_t i = 0;
	for (const auto &[x, y] : source)
	{
		sourceMatrix(0, i) = x;
		sourceMatrix(1, i) = y;
		i++;
	}
	return sourceMatrix;
}

colCoordMat vectorToMatrix(std::vector<coord> source)
{
	// Row 0 is x, row 1 is y
	colCoordMat sourceMatrix(2, source.size());
	for (int i = 0; i < source.size(); i++)
	{
		sourceMatrix(0, i) = source[i].first;
		sourceMatrix(1, i) = source[i].second;
	}
	return sourceMatrix;
}

std::vector<coord> matrixToVector(colCoordMat sourceMatrix)
{
	// Row 0 is x, row 1 is y
	std::vector<coord> source(sourceMatrix.cols(), coord());
	for (int i = 0; i < sourceMatrix.cols(); i++)
	{
		source[i].first = sourceMatrix(0, i);
		source[i].second = sourceMatrix(1, i);
	}
	return source;
}

// To Cartesian Space
Eigen::Vector2f getPoint(const Eigen::Vector2f &coord, const Eigen::Vector2f &origin, const Eigen::Vector2f &v1, const Eigen::Vector2f &v2)
{
	Eigen::Matrix2f mat = Eigen::Matrix2f::Zero();
	mat.col(0) = v1;
	mat.col(1) = v2;

	Eigen::Vector2f result = (mat * coord) + origin;
	return result;
}

// To Hex Space
Eigen::Vector2f getCoords(const Eigen::Vector2f &pt, const Eigen::Vector2f &origin, const Eigen::Vector2f &v1, const Eigen::Vector2f &v2)
{
	Eigen::Matrix2f mat = Eigen::Matrix2f::Zero();
	mat.col(0) = v1;
	mat.col(1) = v2;
	Eigen::Vector2f result = mat.inverse() * (pt - origin);
	return result;
}

Eigen::Vector2i roundPtToCoord(const Eigen::Vector2f &pt, const Eigen::Vector2f &origin, const Eigen::Vector2f &v1, const Eigen::Vector2f &v2)
{
	return getCoords(pt, origin, v1, v2).unaryExpr(static_cast<float (*)(float)>(std::round)).cast<int>();
}
