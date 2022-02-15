#pragma once
#include <string>
#include <vector>
#include <functional>
#include <Eigen/Dense>

typedef std::pair<float, float> coord;
//TODO: make this hold the data directly
struct coord3D {
	float x = 0;
	float y = 0;
	float z = 0;

	coord3D(float x, float y, float z):x(x), y(y), z(z) {};
	coord3D operator+(const coord3D& other)const {
		return coord3D(x + other.x, y + other.y, z + other.z);
	}
};

std::vector<std::pair<std::vector<coord>, std::vector<coord>>> importAlignments(std::string alignmentFile);

void loadTSV(std::string tsvFile, std::vector < std::pair<std::vector<coord>, std::vector<coord>>> srctrgts);


std::function<std::vector<coord>(std::vector<coord>)> getTransSVD(const std::vector<coord>& source, const std::vector<coord>& target); //First is the source, second is the target

//this helper function produces an array of length n, which is all zero except a 1 in the i-th spot.
std::vector<float>getClusterArray(size_t length, size_t i);

Eigen::Matrix<float, 2, 2> getSVDRotation(Eigen::Matrix<float, 2, Eigen::Dynamic> sourceMatrix, Eigen::Matrix<float, 2, Eigen::Dynamic> targetMatrix);


std::vector<coord> growAndCover(std::vector<coord> source, std::vector<coord> bounds, unsigned int, unsigned int);

//TODO: Eigen -> Modern c++ svd linear algebra operations

//Ransac -> initial guess, then refined version