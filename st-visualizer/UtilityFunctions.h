#pragma once
#include <vector>
#include <functional>
#include "UtilityFunctions.h"

#include <string>
#include <vector>
#include <functional>
#include <Eigen/Dense>

//Conventient typedefs

typedef Eigen::Matrix<float, 2, Eigen::Dynamic> colCoordMat;

typedef std::pair<float, float> coord;
//TODO: make this hold the data directly
struct coord3D {
	float x = 0;
	float y = 0;
	float z = 0;

	coord3D(float x, float y, float z) :x(x), y(y), z(z) {};
	coord3D operator+(const coord3D& other)const {
		return coord3D(x + other.x, y + other.y, z + other.z);
	}
};


inline colCoordMat vectorToMatrix(std::vector<coord> source) {
	//Row 0 is x, row 1 is y
	colCoordMat sourceMatrix(2, source.size());
	for (int i = 0; i < source.size(); i++) {
		sourceMatrix(0, i) = source[i].first;
		sourceMatrix(1, i) = source[i].second;
	}
	return sourceMatrix;
}

inline std::vector<coord> matrixToVector(colCoordMat sourceMatrix) {
	//Row 0 is x, row 1 is y
	std::vector<coord> source(sourceMatrix.cols(), coord());
	for (int i = 0; i < sourceMatrix.cols(); i++) {
		source[i].first = sourceMatrix(0, i);
		source[i].second = sourceMatrix(1, i);
	}
	return source;
}



//Vector Mapping Functions and overloads
template<typename T, typename G>
std::vector<G> operator<<(const std::vector<T>& vec, const std::function<G(T)>& op) {
	std::vector<G> c;
	for (const T& item : vec) {
		c.push_back(op(item));
	}
	return c;
}

template<typename T, typename G>
std::vector<G> operator<<(const std::vector<T>& vec, const std::function<G(T, size_t)>& op) {
	std::vector<G> c;
	for (size_t i = 0; i < vec.size(); i++) {
		c.push_back(op(vec[i], i));
	}
	return c;
}

template<typename T, typename G>
std::vector<G> operator<<(const std::vector<T>& vec, const std::function<G(T, int, const std::vector<T>&)>& op) {
	std::vector<G> c;
	for (int i = 0; i < vec.size(); i++) {
		c.push_back(op(vec[i], i, vec));
	}
	return c;
}

template<typename T>
std::vector<T> filter(const std::vector<T>& vec, std::function<bool(T)> op) {
	std::vector<T> res;
	for (const T& item : vec) {
		if (op(item)) { 
			res.push_back(item); 
		}
	}
	return res;
}

template<typename T>
std::vector<T> concat(const std::vector<T>& a, const std::vector<T>& b) {
	std::vector<T> bothNeighbors(a.begin(), a.end());
	bothNeighbors.insert(bothNeighbors.end(), b.begin(), b.end());
	return bothNeighbors;
};

template<typename T>
std::vector<T> operator+(const std::vector<T>& a, const std::vector<T>& b) {
	return concat(a, b);
}

//Runs function over range []
template<typename T>
std::vector<T> table(size_t length, std::function<T(size_t)> op) {
	return std::vector<int>(0, length) << std::function<size_t(int, size_t)>([](int, size_t i) {return i; })
		<< op;
}