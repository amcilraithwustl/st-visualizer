#pragma once
#include <vector>
#include <functional>
#include "UtilityFunctions.h"

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