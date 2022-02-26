#include "GrowAndCover.h"
#include <iostream>

float epsilon = 0.2;

Eigen::Rotation2Df rotM(float a) {
	return Eigen::Rotation2D(a);
}

auto hexM = rotM(PI / 3);

//To Hex Space
//TODO: Make this go to integer
Eigen::Vector2f getCoords(const Eigen::Vector2f& pt,const  Eigen::Vector2f& origin, const Eigen::Vector2f& v1, const Eigen::Vector2f& v2) {
	Eigen::Matrix2f mat = Eigen::Matrix2f::Zero();
	mat.col(0) = v1;
	mat.col(1) = v2;
	Eigen::Vector2f result = mat.inverse() * (pt-origin);
	return result;

}

//To Cartesian Space
Eigen::Vector2f getPoint(const Eigen::Vector2f& coord, const Eigen::Vector2f& origin, const Eigen::Vector2f& v1, const Eigen::Vector2f& v2) {
	Eigen::Matrix2f mat = Eigen::Matrix2f::Zero();
	mat.col(0) = v1;
	mat.col(1) = v2;

	//TODO: I think this can be made much simpler
	Eigen::Vector2f result = (mat * coord ) + origin;
	return result;
}

std::vector<coord> growAndCover(std::vector<coord> source, std::vector<coord> bounds, unsigned int, unsigned int)
{
	return std::vector<coord>();
}