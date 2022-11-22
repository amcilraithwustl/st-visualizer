#pragma once

#include "UtilityFunctions.h"

#define HEX_ROUNDING_ERROR 0.2f

Eigen::Matrix2Xf growAndCover(const Eigen::Matrix2Xf& pts, const Eigen::Matrix2Xf& samples, const unsigned& wid,
                              const unsigned& num);

std::pair<std::vector<int>, Eigen::Matrix2Xi> getInliers(const Eigen::Matrix2Xf& pts, const Eigen::Vector2f& origin,
                                                         const Eigen::Vector2f& v1);

Eigen::Matrix2Xi roundPtsToCoords(const Eigen::Matrix2Xf& pts, const Eigen::Vector2f& origin, const Eigen::Vector2f& v1, const Eigen::Vector2f& v2);

//returns a pair of { 
//	{best origin, best v1},
//	{inlier indices, inlier matrix}
//} where the inlier is built off getInliers
std::pair<std::pair<Eigen::Vector2f, Eigen::Vector2f>, std::pair<std::vector<int>, Eigen::Matrix2Xi>> initGridInliers(
	const Eigen::Matrix2Xf& pts, const int& num);

//Gets the best origin and v1 based on the points and the grid passed in to minimize variance
std::pair<Eigen::Vector2f, Eigen::Vector2f> getGrid(const Eigen::Matrix2Xf& pts, const std::vector<int>& indices,
                                                    const Eigen::Matrix2Xi& intCoords);

//Refine Grid
std::pair<Eigen::Vector2f, Eigen::Vector2f> refineGrid(const Eigen::Matrix2Xf& pts,
                                                       const std::pair<Eigen::Vector2f, Eigen::Vector2f>& grid,
                                                       const std::pair<std::vector<int>, Eigen::Matrix2Xi>& inliers);

std::pair<std::pair<Eigen::Vector2f, Eigen::Vector2f>, Eigen::Matrix2Xi>
getGridAndCoords(const Eigen::Matrix2Xf& pts, const int& num);
