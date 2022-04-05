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

	coord3D(float x, float y, float z) : x(x), y(y), z(z)
	{
	}

	coord3D operator+(const coord3D& other) const
	{
		return coord3D(x + other.x, y + other.y, z + other.z);
	}
};


colCoordMat listToMatrix(std::list<coord> source);

colCoordMat vectorToMatrix(std::vector<coord> source);

std::vector<coord> matrixToVector(colCoordMat sourceMatrix);

//List Mapping Functions and overloads
template <typename T, typename G>
std::list<G> mapList(const std::list<T>& vec, const std::function<G(const T&)>& op)
{
	std::list<G> c;
	for(auto& item : vec)
	{
		c.push_back(op(item));
	}
	return c;
}

template <typename T, typename G>
std::list<G> operator<<(const std::list<T>& vec, const std::function<G(const T&)>& op) { return mapList(vec, op); }

//Vector Mapping Functions and overloads
template <typename T, typename G>
std::vector<G> mapVector(const std::vector<T>& vec, const std::function<G(const T&, size_t)>& op)
{
	std::vector<G> c;
	c.reserve(vec.size());
	for(size_t i = 0; i < vec.size(); i++)
	{
		c.push_back(op(vec[i], i));
	}
	return c;
}

template <typename T, typename G>
std::vector<G> mapVector(const std::vector<T>& vec, const std::function<G(const T&)>& op)
{
	std::function<G(const T&, size_t)> new_op([op](const T& val, size_t) {return op(val); });
	return mapVector(vec, new_op);
}

template <typename T, typename G>
std::vector<G> mapVector(const std::vector<T>& vec, const std::function<G(T)>& op)
{
	std::function<G(const T&, size_t)> new_op([op](const T& val, size_t){return op(val);});
	return mapVector(vec, new_op);
}

template <typename T, typename G>
std::vector<G> mapVector(const std::vector<T>& vec, const std::function<G(T, size_t)>& op)
{
	std::function<G(const T&, size_t)> new_op([op](const T& val, size_t i) {return op(val, i); });
	return mapVector(vec, new_op);
}

//Operator overloads. First two are by reference functions, second two are by copy functions
template <typename T, typename G>
std::vector<G> operator<<(const std::vector<T>& vec, const std::function<G(const T&)>& op){return mapVector(vec, op);}

template <typename T, typename G>
std::vector<G> operator<<(const std::vector<T>& vec, const std::function<G(const T&, size_t)>& op)
{
	return mapVector(vec, op);
}

template <typename T, typename G>
std::vector<G> operator<<(const std::vector<T>& vec, const std::function<G(T)>& op) { return mapVector(vec, op); }

template <typename T, typename G>
std::vector<G> operator<<(const std::vector<T>& vec, const std::function<G(T, size_t)>& op)
{
	return mapVector(vec, op);
}

//mapThread and its overloads
template <typename A, typename B, typename C>
std::vector<C> mapThread(const std::vector<A>& vec1, const std::vector<B>& vec2, const std::function<C(const A&, const B&, size_t)>& op)
{
	if (vec1.size() != vec2.size()) throw "SIZE MISMATCH";

	std::vector<C> new_vector;
	new_vector.reserve(vec1.size());
	for(size_t i = 0; i < vec1.size(); i++)
	{
		new_vector.push_back(op(vec1[i], vec2[i], i));
	}
	return new_vector;
}

template <typename A, typename B, typename C>
std::vector<C> mapThread(const std::vector<A>& vec1, const std::vector<B>& vec2, const std::function<C(A, B, size_t)>& op)
{
	return mapThread(vec1, vec2, std::function([&](const A& a, const B& b, size_t i) {return op(a, b, i); }));
}

template <typename A, typename B, typename C>
std::vector<C> mapThread(const std::vector<A>& vec1, const std::vector<B>& vec2, const std::function<C(const A&, const B&)>& op)
{
	return mapThread(vec1, vec2, std::function([&](const A& a, const B& b, size_t) {return op(a, b); }));
}

template <typename A, typename B, typename C>
std::vector<C> mapThread(const std::vector<A>& vec1, const std::vector<B>& vec2, const std::function<C(A, B)>& op)
{
	return mapThread(vec1, vec2, std::function([&](const A& a, const B& b, size_t) {return op(a, b); }));
}

//To Cartesian Space
Eigen::Vector2f getPoint(const Eigen::Vector2f& coord, const Eigen::Vector2f& origin, const Eigen::Vector2f& v1,
                         const Eigen::Vector2f& v2);

//To Hex Space
Eigen::Vector2f getCoords(const Eigen::Vector2f& pt, const Eigen::Vector2f& origin, const Eigen::Vector2f& v1,
                          const Eigen::Vector2f& v2);

Eigen::Vector2i roundPtToCoord(const Eigen::Vector2f& pt, const Eigen::Vector2f& origin, const Eigen::Vector2f& v1,
                               const Eigen::Vector2f& v2);

template <typename T, typename G>
std::vector<G> operator<<(const std::vector<T>& vec, const std::function<G(T, int, const std::vector<T>&)>& op)
{
	std::vector<G> c;
	for(int i = 0; i < vec.size(); i++)
	{
		c.push_back(op(vec[i], i, vec));
	}
	return c;
}

template <typename T>
std::vector<T> filter(const std::vector<T>& vec, std::function<bool(const T&)> op)
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

template < typename T>
std::list<T> filter(const std::list<T>& vec, std::function<bool(const T&)> op)
{
	std::list<T> res;
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
