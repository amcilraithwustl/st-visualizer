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


//Vector Mapping Functions and overloads
template <typename T, typename G>
std::vector<G> operator<<(const std::vector<T>& vec, const std::function<G(T)>& op)
{
	std::vector<G> c;
	for (const T& item : vec)
	{
		c.push_back(op(item));
	}
	return c;
}

template <typename T, typename G>
std::vector<G> operator<<(const std::vector<T>& vec, const std::function<G(T, size_t)>& op)
{
	std::vector<G> c;
	for (size_t i = 0; i < vec.size(); i++)
	{
		c.push_back(op(vec[i], i));
	}
	return c;
}

//To Cartesian Space
inline Eigen::Vector2f getPoint(const Eigen::Vector2f& coord, const Eigen::Vector2f& origin, const Eigen::Vector2f& v1,
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
	const auto base_delta = getPoint(getCoords(pt, origin, v1, v2).cast<int>().cast<float>(), origin, v1, v2) - pt;
	//Find distance of the floor value of the int
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
		if (distances[minDist] > distances[i])
		{
			minDist = i;
		}
	}

	switch (minDist)
	{
	case 0:
		return getCoords(pt + base_delta, origin, v1, v2).cast<int>();
	case 1:
		return getCoords(pt + right_delta, origin, v1, v2).cast<int>();
	case 2:
		return getCoords(pt + left_delta, origin, v1, v2).cast<int>();
	default:
		return getCoords(pt + front_delta, origin, v1, v2).cast<int>();
	}
}

template <typename T, typename G>
std::vector<G> operator<<(const std::vector<T>& vec, const std::function<G(T, int, const std::vector<T>&)>& op)
{
	std::vector<G> c;
	for (int i = 0; i < vec.size(); i++)
	{
		c.push_back(op(vec[i], i, vec));
	}
	return c;
}

template <typename T>
std::vector<T> filter(const std::vector<T>& vec, std::function<bool(T)> op)
{
	std::vector<T> res;
	for (const T& item : vec)
	{
		if (op(item))
		{
			res.push_back(item);
		}
	}
	return res;
}

template <typename T>
std::vector<T> concat(const std::vector<T>& a, const std::vector<T>& b)
{
	std::vector<T> bothNeighbors(a.begin(), a.end());
	bothNeighbors.insert(bothNeighbors.end(), b.begin(), b.end());
	return bothNeighbors;
};

template <typename T>
std::vector<T> operator+(const std::vector<T>& a, const std::vector<T>& b)
{
	return concat(a, b);
}

//Runs function over range []
template <typename T>
std::vector<T> table(size_t length, std::function<T(size_t)> op)
{
	return std::vector<int>(0, length) << std::function<size_t(int, size_t)>([](int, size_t i) { return i; })
		<< op;
}
