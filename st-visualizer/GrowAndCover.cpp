#include "GrowAndCover.h"
#include <iostream>
#include <math.h>

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

//Pull out all of the points which lie on the grid. Only works in hex space. 
// v2 is pi/3 radians from v1 counterclockwise with the same magnitude.
std::pair<std::vector<int>, Eigen::Matrix2Xi> getInliers(Eigen::Matrix2Xf pts, Eigen::Vector2f origin, Eigen::Vector2f v1)
{
	Eigen::Vector2f v2 = hexM * v1;
	Eigen::Matrix2Xi intcoords = Eigen::Matrix2Xi::Zero(2, pts.cols());
	for (size_t i = 0; i < pts.cols(); i++) {
		//Get the coordinate, round it to the nearest value, cast it to an integer, then save it in the intcoords matrix
		Eigen::Vector2i rounded = getCoords(pts.col(i), origin, v1, v2).unaryExpr([](float i) {return std::round(i); }).cast<int>();
		intcoords.col(i) = rounded;
	}

	//Check which indices are within the appropriate bounds
	std::vector<int> indices;
	for (int i = 0; i < pts.cols(); i++) {
		Eigen::Vector2f pt = pts.col(i);
		Eigen::Vector2f returnPt = getPoint(intcoords.col(i).cast<float>(), origin, v1, v2);
		//std::cout << "TEST\n" << returnPt << std::endl << std::endl << pt << std::endl;
		float errorMargin = HEX_ROUNDING_ERROR * v1.norm();
		float realMargin = (pt - returnPt).norm();
		if(realMargin < errorMargin) {
			indices.push_back(i);
		}
	}

	Eigen::Matrix2Xi revisedCoords = Eigen::Matrix2Xi::Zero(2, indices.size());
	for (int i = 0; i < indices.size(); i++) {
		revisedCoords.col(i) = intcoords.col(indices[i]);
	}


	return std::pair<std::vector<int>, Eigen::Matrix2Xi>(indices, revisedCoords);
}

//returns {best origin, best v1},resulting inliers
std::pair<std::pair<Eigen::Vector2f, Eigen::Vector2f>, std::pair<std::vector<int>, Eigen::Matrix2Xi>> initGridInliers(Eigen::Matrix2Xf pts, int num) {
	if (pts.cols() < 2) throw("Need more points to test");

	std::pair<std::vector<int>, Eigen::Matrix2Xi> bestInliers;
	Eigen::Vector2f bestOrigin({ 0,0 });
	Eigen::Vector2f bestV1({ 1,0 });

	for (int i = 0; i < num; i++) {
		//Pick a random point
		size_t originIndex = rand() % pts.cols();
		Eigen::Vector2f randOri = pts.col(originIndex);

		//Get the next closest point
		Eigen::Matrix2Xf adjustedPoints = pts.colwise() - randOri;
		int selectedPoint = originIndex > 0 ? 0 : 1; //I am assuming that there are more than two points. TODO: Add edge case checking.
		float minNorm = adjustedPoints.col(selectedPoint).norm();
		for (int j = 0; j < adjustedPoints.cols(); j++) {
			float testNorm = adjustedPoints.col(j).norm();
			if (j != originIndex && testNorm < minNorm) {
				selectedPoint = j;
				minNorm = testNorm;
			}
		}

		Eigen::Vector2f v1 = adjustedPoints.col(selectedPoint);


		//See how many inliers there are with the resulting grid
		auto inliers = getInliers(pts, randOri, v1);

		//If its a better fit, save it
		if (inliers.first.size() > bestInliers.first.size()) {
			bestInliers = inliers;
			bestOrigin = randOri;
			bestV1 = v1;
		}

	}

	return std::pair<
		std::pair<Eigen::Vector2f, Eigen::Vector2f>,
		std::pair<std::vector<int>, Eigen::Matrix2Xi>
	>(std::pair<Eigen::Vector2f, Eigen::Vector2f>(bestOrigin, bestV1), bestInliers);
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