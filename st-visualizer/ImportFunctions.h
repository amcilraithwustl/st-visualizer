#pragma once
#include <string>
#include <vector>
#include <functional>

typedef std::pair<float, float> coord;
struct coord3D {
	float x = 0;
	float y = 0;
	float z = 0;
	coord3D(float x, float y, float z):x(x), y(y), z(z) {};
	coord3D operator+(const coord3D& other)const {
		return coord3D(x + other.x, y + other.y, z + other.z);
	}
};

int getX(coord c) {
	return c.first;
}
int getY(coord c) {
	return c.second;
}

std::vector<std::pair<std::vector<coord>, std::vector<coord>>> importAlignments(std::string alignmentFile);

void loadTSV(std::string tsvFile, std::vector < std::pair<std::vector<coord>, std::vector<coord>>> srctrgts);


std::function<std::vector<coord>(std::vector<coord>)> getTransSVD(const std::pair<std::vector<coord>, std::vector<coord>>& alignment); //First is the source, second is the target

//this helper function produces an array of length n, which is all zero except a 1 in the i-th spot.
std::vector<float>getClusterArray(size_t length, size_t i) {
	
	std::vector<float>ret(length, 0);
	if (i < length) {
		ret[i] = 1;
	}
	return ret;
}

std::vector<coord> growAndCover(std::vector<coord> source, std::vector<coord> bounds, unsigned int, unsigned int);

//TODO: Eigen -> Modern c++ svd linear algebra operations

//Ransac -> initial guess, then refined version