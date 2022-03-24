#pragma once
#include <string>
#include <vector>
#include <functional>
#include "UtilityFunctions.h"

std::vector<std::pair<std::vector<coord>, std::vector<coord>>> importAlignments(const std::string& alignment_file);

struct tsv_return_type
{
	std::vector<std::string> names;
	std::vector<Eigen::Matrix3Xf> slices;
	std::vector<std::vector<std::vector<float>>> clusters;
	std::vector<std::vector<std::vector<float>>> values;
};


tsv_return_type loadTsv(const std::string& file_name, const std::vector<std::string>& slice_names,
                      unsigned int slice_index, unsigned int tissue_index, std::pair<
	                      unsigned, unsigned> row_col_indices, unsigned int cluster_ind, const std
                      ::vector<unsigned>& feature_indices, unsigned int z_distance,
                      std::vector<std::pair<std::vector<coord>, std::vector<coord>>> source_targets);


std::function<std::vector<coord>(std::vector<coord>)> getTransSVD(const std::vector<coord>& source,
                                                                  const std::vector<coord>& target);
//First is the source, second is the target

//this helper function produces an array of length n, which is all zero except a 1 in the i-th spot.
std::vector<float> getClusterArray(size_t length, size_t i);

Eigen::Matrix2f getSVDRotation(colCoordMat source_matrix, colCoordMat target_matrix);

colCoordMat translateToZeroCentroid(colCoordMat source_matrix);
