#pragma once
#include <string>
#include <vector>
#include <functional>
#include "UtilityFunctions.h"

std::vector<std::pair<std::vector<coord>, std::vector<coord>>> importAlignments(const std::string& alignmentFile);

struct tsvReturnType {
	std::vector<std::string> names;
	std::vector<Eigen::Matrix3Xf> slices;
	std::vector<std::vector<std::vector<float>>> clusters;
	std::vector<std::vector<std::vector<float>>> vals;
};


tsvReturnType loadTsv(std::string fname, std::vector<std::string> sliceNames, unsigned int sliceInd, unsigned int tissueInd, std::pair<
             unsigned, unsigned> rowcolInds, unsigned int clusterInd, std
             ::vector<unsigned> featureInds, unsigned int zDis, std::vector<std::pair<std::vector<coord>, std::vector<coord>>> srctgts);


std::function<std::vector<coord>(std::vector<coord>)> getTransSVD(const std::vector<coord>& source, const std::vector<coord>& target); //First is the source, second is the target

//this helper function produces an array of length n, which is all zero except a 1 in the i-th spot.
std::vector<float>getClusterArray(size_t length, size_t i);

Eigen::Matrix2f getSVDRotation(colCoordMat sourceMatrix, colCoordMat targetMatrix);

colCoordMat translateToZeroCentroid(colCoordMat sourceMatrix);

//TODO: Eigen -> Modern c++ svd linear algebra operations

//Ransac ->initial guess, then refined version