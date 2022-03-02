#include "GrowAndCover.h"
#include <iostream>

Eigen::Rotation2Df rotM(float a) {
	return Eigen::Rotation2D(a);
}

auto hexM = rotM(PI / 3);

//To Hex Space
Eigen::Vector2f getCoords(const Eigen::Vector2f& pt,const  Eigen::Vector2f& origin, const Eigen::Vector2f& v1, const Eigen::Vector2f& v2) {
	Eigen::Matrix2f mat = Eigen::Matrix2f::Zero();
	mat.col(0) = v1;
	mat.col(1) = v2;
	Eigen::Vector2f result = mat.inverse() * (pt-origin);
	return result;

}

std::pair<std::vector<int>, Eigen::Matrix2Xi> getInliers(Eigen::Matrix2Xf pts, Eigen::Vector2f origin, Eigen::Vector2f v1)
{
	Eigen::Vector2f v2 = hexM * v1;
	Eigen::Matrix2Xi intcoords = Eigen::Matrix2Xi::Zero(2, pts.cols());
	for (size_t i = 0; i < pts.cols(); i++) {
		//Get the coordinate, round it to the nearest value, cast it to an integer, then save it in the intcoords matrix
		Eigen::Vector2i rounded = getCoords(pts.col(i), origin, v1, v2).array().round().cast<int>();
		intcoords.col(i) = rounded;
	}


	//Check which indices are within the appropriate bounds
	std::vector<int> indices;
	for (int i = 0; i < pts.cols(); i++) {
		Eigen::Vector2f pt = pts.col(i);
		Eigen::Vector2f returnPt = getPoint(intcoords.col(i).cast<float>(), origin, v1, v2);
		if (pt.isApprox(returnPt, HEX_ROUNDING_ERROR * v1.norm())) {
			indices.push_back(i);
		}
	}

	Eigen::Matrix2Xi revisedCoords = Eigen::Matrix2Xi::Zero(2, indices.size());
	for (int i = 0; i < indices.size(); i++) {
		revisedCoords.col(i) = intcoords.col(indices[i]);
	}


	return std::pair<std::vector<int>, Eigen::Matrix2Xi>(indices, revisedCoords);
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