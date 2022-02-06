#pragma once
#include <vector>
#include <functional>
#include "UtilityFunctions.h"

//Vector Mapping Functions
template<typename T, typename G>
std::vector<G> operator<<(std::vector<T> vec, const std::function<G(T)>& op) {
	std::vector<G> c;
	for (T& item : vec) {
		c.push_back(op(item));
	}
	return c;
}

template<typename T>
std::vector<T> filter(std::vector<T>vec, std::function<bool(T)> op) {
	std::vector<T> res;
	for (T& item : vec) {
		if (op(item)) { 
			res.push_back(item); 
		}
	}
	return res;
}