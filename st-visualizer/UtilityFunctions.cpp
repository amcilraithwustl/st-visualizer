#pragma once
#include <vector>
#include <functional>
#include "UtilityFunctions.h"
#include <vector>
#include <functional>
#include <Eigen/Dense>


colCoordMat vectorToMatrix(std::vector<coord> source)
{
	//Row 0 is x, row 1 is y
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
	//Row 0 is x, row 1 is y
	std::vector<coord> source(sourceMatrix.cols(), coord());
	for (int i = 0; i < sourceMatrix.cols(); i++)
	{
		source[i].first = sourceMatrix(0, i);
		source[i].second = sourceMatrix(1, i);
	}
	return source;
}




//To Cartesian Space
Eigen::Vector2f getPoint(const Eigen::Vector2f& coord, const Eigen::Vector2f& origin, const Eigen::Vector2f& v1,
                                const Eigen::Vector2f& v2)
{
	Eigen::Matrix2f mat = Eigen::Matrix2f::Zero();
	mat.col(0) = v1;
	mat.col(1) = v2;

	Eigen::Vector2f result = (mat * coord) + origin;
	return result;
}

//To Hex Space
Eigen::Vector2f getCoords(const Eigen::Vector2f& pt, const Eigen::Vector2f& origin, const Eigen::Vector2f& v1,
                          const Eigen::Vector2f& v2)
{
	Eigen::Matrix2f mat = Eigen::Matrix2f::Zero();
	mat.col(0) = v1;
	mat.col(1) = v2;
	Eigen::Vector2f result = mat.inverse() * (pt - origin);
	return result;
}

Eigen::Vector2i roundPtToCoord(const Eigen::Vector2f& pt, const Eigen::Vector2f& origin, const Eigen::Vector2f& v1,
	const Eigen::Vector2f& v2)
{

	const auto base_point = getPoint(getCoords(pt, origin, v1, v2).cast<int>().cast<float>(), origin, v1, v2);
	return getCoords(pt, origin, v1, v2).unaryExpr(static_cast<float(*)(float)>(std::round)).cast<int>();
	const auto base_delta = base_point - pt;
	const auto right_delta = base_delta + v1;
	const auto left_delta = base_delta + v2;
	const auto front_delta = base_delta + v1 + v2;
	constexpr int num_pts = 4;
	const float distances[num_pts] = {
		base_delta.squaredNorm(), right_delta.squaredNorm(), left_delta.squaredNorm(), front_delta.squaredNorm()
	};
	int minDist = 0;
	for (int i = 1; i < num_pts; i++)
	{
		if (distances[i] < distances[minDist])
		{
			minDist = i;
		}
	}

	switch (minDist)
	{
	case 0:
		return getCoords(pt + base_delta, origin, v1, v2).unaryExpr(static_cast<float(*)(float)>(std::round)).cast<int>();
	case 1:
		return getCoords(pt + right_delta, origin, v1, v2).unaryExpr(static_cast<float(*)(float)>(std::round)).cast<int>();
	case 2:
		return getCoords(pt + left_delta, origin, v1, v2).unaryExpr(static_cast<float(*)(float)>(std::round)).cast<int>();
	default:
		return getCoords(pt + front_delta, origin, v1, v2).unaryExpr(static_cast<float(*)(float)>(std::round)).cast<int>();
	}
}
