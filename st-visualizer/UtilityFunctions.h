#pragma once
#include <vector>
#include <functional>
#include "UtilityFunctions.h"
#include <vector>
#include <functional>
#include <Eigen/Dense>


constexpr float pi = static_cast<float>(3.1415926535);

//Convenient typedefs

using colCoordMat = Eigen::Matrix2Xf;

using coord = std::pair<float, float>;

//TODO: make this hold the data directly
struct coord3D
{
	float x = 0;
	float y = 0;
	float z = 0;

	coord3D(float x, float y, float z) : x(x), y(y), z(z){}

	coord3D operator+(const coord3D& other) const
	{
		return coord3D(x + other.x, y + other.y, z + other.z);
	}
};


colCoordMat vectorToMatrix(std::vector<coord> source);

std::vector<coord> matrixToVector(colCoordMat sourceMatrix);

//Vector Mapping Functions and overloads
template <typename T, typename G>
std::vector<G> operator<<(const std::vector<T>& vec, const std::function<G(T)>& op);
template <typename T, typename G>
std::vector<G> operator<<(const std::vector<T>& vec, const std::function<G(T, size_t)>& op);

//To Cartesian Space
Eigen::Vector2f getPoint(const Eigen::Vector2f& coord, const Eigen::Vector2f& origin, const Eigen::Vector2f& v1,
                         const Eigen::Vector2f& v2);

//To Hex Space
Eigen::Vector2f getCoords(const Eigen::Vector2f& pt, const Eigen::Vector2f& origin, const Eigen::Vector2f& v1,
                          const Eigen::Vector2f& v2);

Eigen::Vector2i roundPtToCoord(const Eigen::Vector2f& pt, const Eigen::Vector2f& origin, const Eigen::Vector2f& v1,
                               const Eigen::Vector2f& v2);

template <typename T, typename G>
std::vector<G> operator<<(const std::vector<T>& vec, const std::function<G(T, int, const std::vector<T>&)>& op);

template <typename T>
std::vector<T> filter(const std::vector<T>& vec, std::function<bool(T)> op);

template <typename T>
std::vector<T> concat(const std::vector<T>& a, const std::vector<T>& b);

template <typename T>
std::vector<T> operator+(const std::vector<T>& a, const std::vector<T>& b);

//Runs function over range []
template <typename T>
std::vector<T> table(size_t length, std::function<T(size_t)> op);
