#pragma once

#include "UtilityFunctions.h"

#define HEX_ROUNDING_ERROR 0.2f

std::vector<coord> growAndCover(std::vector<coord> source, std::vector<coord> bounds, unsigned int, unsigned int);

Eigen::Vector2f getPoint(const Eigen::Vector2f& coord, const Eigen::Vector2f& origin, const Eigen::Vector2f& v1, const Eigen::Vector2f& v2);

Eigen::Vector2f getCoords(const Eigen::Vector2f& pt, const  Eigen::Vector2f& origin, const Eigen::Vector2f& v1, const Eigen::Vector2f& v2);

std::pair<std::vector<int>, Eigen::Matrix2Xi> getInliers(Eigen::Matrix2Xf pts, Eigen::Vector2f origin, Eigen::Vector2f v1);