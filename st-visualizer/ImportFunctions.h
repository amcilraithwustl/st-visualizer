#pragma once
#include <string>
#include <vector>
#include <functional>
#include <Eigen/Dense>
#include "UtilityFunctions.h"

std::vector<std::pair<std::vector<coord>, std::vector<coord>>> importAlignments(std::string alignmentFile);

void loadTSV(std::string tsvFile, std::vector < std::pair<std::vector<coord>, std::vector<coord>>> srctrgts);


std::function<std::vector<coord>(std::vector<coord>)> getTransSVD(const std::vector<coord>& source, const std::vector<coord>& target); //First is the source, second is the target

//this helper function produces an array of length n, which is all zero except a 1 in the i-th spot.
std::vector<float>getClusterArray(size_t length, size_t i);

Eigen::Matrix2f getSVDRotation(colCoordMat sourceMatrix, colCoordMat targetMatrix);

colCoordMat translateToZeroCentroid(colCoordMat sourceMatrix);

//TODO: Eigen -> Modern c++ svd linear algebra operations

//Ransac ->initial guess, then refined version