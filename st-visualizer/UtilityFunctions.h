#pragma once
#include <vector>
#include <functional>
#include "UtilityFunctions.h"
#include <vector>
#include <functional>
#include <Eigen/Dense>
#include "triangle-1.6/triangle.h"
#include <iostream>

#include "JSONParser.h"


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
std::vector<C> mapThread(const std::vector<A>& vec1, const std::vector<B>& vec2, const std::function<C(const A&, const B&)>& op)
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
	return std::vector(length, 0)
    << std::function<size_t(int, size_t)>([](int, size_t i) { return i; })
		<< op;
}


//Functions for ease of use with triangle
inline Eigen::Vector2f getCornerVector(const triangulateio& obj, int corner_index)
{
	const auto x_index = corner_index * 2;
	return Eigen::Vector2d(obj.pointlist[x_index], obj.pointlist[x_index + 1]).cast<float>();
}

inline std::vector<int> getTriangleCornerIndices(const triangulateio& obj, int triangleIndex)
{
	const int* ptr = obj.trianglelist + static_cast<ptrdiff_t>(triangleIndex * obj.numberofcorners);
	return { ptr, ptr + obj.numberofcorners };
}

inline Eigen::Matrix2Xf getTriangleMatrix(const triangulateio& obj, int triangleIndex)
{
	const auto corners = getTriangleCornerIndices(obj, triangleIndex);

	Eigen::Matrix2Xf ret = Eigen::Matrix2Xf::Zero(2, corners.size());

	for (size_t j = 0; j < corners.size(); j++)
	{
		ret.col(static_cast<Eigen::Index>(j)) = getCornerVector(obj, corners[j]);
	}
	return ret;
}

/**
 * \brief 
 * \param mat 
 * \return 
 */
inline triangulateio triangulateMatrix(const Eigen::Matrix2Xf& mat)
{
	//TODO: Investigate flags further
	std::string flags =
		std::string("z") //Start arrays at 0
		// + std::string("V") //Verbose
	+ std::string("Q") //Quiet
		;

	const auto numPoints = mat.cols();
	double* points = new double[numPoints * 2];
	for (int i = 0; i < mat.cols(); i++)
	{
		points[i*2] = mat.col(i)(0);
		points[i*2+1] = mat.col(i)(1);
	}
	triangulateio in = {};
	in.pointlist = points;
	in.numberofpoints = static_cast<int>(numPoints);
	in.numberofpointattributes = 0;
	in.pointmarkerlist = nullptr; //Might be able to use this to associate points with indices in the original matrix
	in.pointattributelist = nullptr; //Might be able to use this to associate points with indices in the original matrix

	triangulateio out = {};
	out.pointlist = nullptr;
	out.trianglelist = nullptr;
	out.pointmarkerlist = nullptr;
	out.pointattributelist = nullptr;
	out.trianglelist = nullptr;

	triangulate(&flags[0], &in, &out, nullptr);

	return out;
}

using json = nlohmann::json;
template<typename T, int G>
std::vector<T> eigenToVec(Eigen::Vector<T, G> mat) { return std::vector<T>(mat.data(), mat.data() + mat.rows()); }

template<typename T>
json toJson(std::vector<T> v)
{
	auto a = json::array();
	for (size_t i = 0; i < v.size(); i++)
	{
		a.push_back(v[i]);
	}
	return a;
}

inline json extractTriangleMathematicaMesh(const triangulateio& obj)
{
	const auto points = table(obj.numberofpoints, std::function([obj](size_t i)
		{
			return getCornerVector(obj, i);
		}));
	const auto triangles = table(obj.numberoftriangles, std::function([obj](size_t i) {return getTriangleCornerIndices(obj, i); }));


	json pointJson = json::array();
	for (auto& pt : points)
	{
		pointJson.push_back(toJson(eigenToVec(pt)));
	}

	json triangleJson = json::array();
	for (auto& set : triangles)
	{
		triangleJson.push_back(toJson(set));
	}
	json ret = json::array({ pointJson ,triangleJson });
	return ret;
};