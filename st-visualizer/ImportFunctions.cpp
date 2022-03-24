#include "ImportFunctions.h"
#include "UtilityFunctions.h"
#include <fstream>
#include <iostream>
#include <functional>
#include <iostream>
#include <algorithm>
#include <limits>
#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <utility>

#include "GrowAndCover.h"
using std::vector;
using std::string;

constexpr int wid_buffer = 2;
constexpr int num_ransac = 5;

//From https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
std::vector<string> splitString(const string& s, const string& delimiter)
{
	size_t pos_start = 0, pos_end;
	const size_t delimiter_length = delimiter.length();
	std::vector<string> res;

	while ((pos_end = s.find(delimiter, pos_start)) != string::npos)
	{
		string token = s.substr(pos_start, pos_end - pos_start);
		pos_start = pos_end + delimiter_length;
		res.push_back(token);
	}

	res.push_back(s.substr(pos_start));
	return res;
}

std::vector<coord> extractCoordinateSet(std::vector<float> top, std::vector<float> bottom)
{
	if (top.size() != bottom.size()) throw std::exception("Size Mismatch");
	std::vector<coord> a;
	a.reserve(top.size());
	for (size_t i = 0; i < top.size(); i++)
	{
		a.emplace_back(coord(top[i], bottom[i]));
	}
	return a;
}

/// Any impossible fields default to inf
vector<float> convertRowToFloat(vector<string> input) {
	return input << std::function([](string s)
		{
			try
			{
				return std::stof(s);
			}
			catch (...)
			{
				std::cerr << "WARNING: Invalid std::stoi attempt. Please check data: " << s << std::endl;
			}
			return std::numeric_limits<float>::infinity();
		}
	);
}

std::vector<std::pair<std::vector<coord>, std::vector<coord>>> importAlignments(const string& alignment_file)
{
	//Import raw file data
	std::ifstream aFile(alignment_file);
	std::vector<string> lines;
	if (aFile.is_open())
	{
		string line;
		while (std::getline(aFile, line))
		{
			lines.push_back(line);
		}
		aFile.close();
	}

	//Split into cells
	vector<vector<string>> csvCells;
	for (const string& row : lines)
	{
		vector<string> rowCells = splitString(row, ",");
		//Drop the first three columns
		csvCells.emplace_back(rowCells.begin() + 3, rowCells.end());
	}
	//Drop the first row
	csvCells = vector(csvCells.begin() + 1, csvCells.end());

	

	// Transform cells into ints
	auto transformCells = csvCells << std::function(convertRowToFloat);

	//Now we have n columns and m*4 rows of ints
	//This part transforms that into an m by 2 by n structure of coords

	//Transpose pairs
	vector<vector<coord>> coordinateSet;
	for (size_t i = 0; i < transformCells.size(); i += 2)
	{
		coordinateSet.push_back(extractCoordinateSet(transformCells[i], transformCells[i + 1]));
	}

	vector<std::pair<vector<coord>, vector<coord>>> finalSet;
	//line up in pairs 
	for (size_t i = 0; i < coordinateSet.size(); i += 2)
	{
		finalSet.emplace_back(coordinateSet[i], coordinateSet[i + 1]);
	}

	return finalSet;
}

tsv_return_type loadTsv(const std::string& file_name, const std::vector<std::string>& slice_names, unsigned int slice_index,
                      unsigned int tissue_index,
                      std::pair<
	                      unsigned, unsigned> row_col_indices, unsigned int cluster_ind,
                      const std::vector<unsigned>& feature_indices, unsigned int z_distance,
                      std::vector<std::pair<std::vector<coord>, std::vector<coord>>> source_targets)
{
	//Import raw file data
	std::ifstream aFile(file_name);
	std::vector<string> lines;
	if (aFile.is_open())
	{
		string line;

		while (std::getline(aFile, line))
		{
			lines.push_back(line);
		}
		aFile.close();
	}
	std::vector<std::vector<std::string>> rawData;
	rawData.reserve(lines.size());
	for (auto& line : lines)
	{
		rawData.push_back(splitString(line, "\t"));
	}

	vector<string> names;
	names.reserve(feature_indices.size());
	for (auto index : feature_indices)
	{
		names.push_back(rawData[0][index]);
	}
	names.emplace_back("No Tissue");

	vector tab(rawData.begin() + 1, rawData.end());

	unsigned int max = 0;
	for (auto row : tab)
	{
		try
		{
			int n = std::stoi(row[0]);
			if (n > static_cast<int>(max)) max = n;
		}
		catch (...)
		{
		}
	}

	unsigned int newClusters = max + 1;
	auto newFeatures = feature_indices.size();

	vector<vector<vector<string>>> records;
	for (const auto& name : slice_names)
	{
		vector<vector<string>> temp;
		for (auto row : tab)
		{
			if (row[slice_index] == name && row[cluster_ind] == "1")
			{
				temp.push_back(row);
			}
		}
		records.push_back(temp);
	}

	vector<Eigen::Matrix2Xf> slices;
	std::pair xy_indices(row_col_indices.second, row_col_indices.first);
	for (size_t i = 0; i < records.size(); i++)
	{
		auto& record = records[i];

		vector<std::pair<float, float>> temp;
		for (auto& row : record)
		{
			std::pair point(std::stof(row[xy_indices.first]), std::stof(row[xy_indices.second]));
			temp.push_back(point);
		}
		if (i == 0)
		{
			slices.push_back(vectorToMatrix(temp));
		}
		else
		{
			std::function transform = getTransSVD(source_targets[i - 1].first, source_targets[i - 1].second);
			slices.push_back(vectorToMatrix(transform(temp)));
		}
	}

	vector<vector<vector<float>>> clusters;
	for (const auto& record : records)
	{
		vector<vector<float>> temp;
		for (const auto& row : record)
		{
			if (row[tissue_index] == "0")
			{
				temp.push_back(getClusterArray(newClusters + 1, newClusters + 1));
			}
			else
			{
				temp.push_back(getClusterArray(newClusters + 1, std::stoi(row[cluster_ind]) + 1));
			}
		}
		clusters.push_back(temp);
	}

	vector<vector<vector<float>>> values;
	for (const auto& record : records)
	{
		vector<vector<float>> temp;
		for (const auto& row : record)
		{
			if (row[tissue_index] == "0")
			{
				temp.push_back(getClusterArray(newFeatures + 1, newFeatures + 1));
			}
			else
			{
				vector<float> a;
				a.reserve(feature_indices.size());
				for (auto ind : feature_indices)
				{
					a.push_back(std::stof(row[ind]));
				}
				a.emplace_back(0);
				temp.push_back(a);
			}
		}
		values.push_back(temp);
	}

	//Add buffer to each slice and grow and cover neighboring slices
	vector<Eigen::Matrix2Xf> new_slices; //Holds the points that we are growing into
	new_slices.reserve(slices.size() + 2);

	for (size_t i = 0; i < slices.size(); i++)
	{
		Eigen::Matrix2Xf result;
		if (i == 0)
		{
			result = growAndCover(slices[i], slices[i + 1], wid_buffer, num_ransac);
		}
		else if (i == slices.size() - 1)
		{
			result = growAndCover(slices[i], slices[i - 1], wid_buffer, num_ransac);
		}
		else
		{
			Eigen::Matrix2Xf temp(2, slices[i + 1].cols() + slices[i - 1].cols());
			temp << slices[i + 1], slices[i - 1];
			result = growAndCover(slices[i], temp, wid_buffer, num_ransac);
		}
		new_slices.push_back(result);
	}

	vector<Eigen::Matrix3Xf> slices3d;
	slices3d.reserve(new_slices.size());
	for (size_t i = 0; i < new_slices.size(); i++)
	{
		Eigen::Matrix2Xf layer2d(2, new_slices[i].cols() + slices[i].cols());
		layer2d << new_slices[i], slices[i]; //The new and old points on that layer
		Eigen::Matrix3Xf layer3d(3, layer2d.cols());
		for (int j = 0; j < layer2d.cols(); j++)
		{
			layer3d.col(j)(0) = layer2d.col(j)(0);
			layer3d.col(j)(1) = layer2d.col(j)(1);
			layer3d.col(j)(2) = static_cast<float>(z_distance * i);
		}
		slices3d.push_back(layer3d);
	}

	//Associating data with the new points on their respective slices
	vector<vector<vector<float>>> grownClusters;
	for (size_t i = 0; i < clusters.size(); i++)
	{
		auto p_1 = clusters[i];
		auto p_2 = new_slices[i];

		vector tempVector(p_2.cols(), getClusterArray(newClusters + 1, newClusters));

		p_1.insert(p_1.end(), tempVector.begin(), tempVector.end());
		grownClusters.push_back(p_1);
	}

	vector<vector<vector<float>>> grown_values;
	for (size_t i = 0; i < values.size(); i++)
	{
		auto p_1 = values[i];
		auto p_2 = new_slices[i];

		vector tempVector(p_2.cols(), getClusterArray(newFeatures + 1, newFeatures));

		p_1.insert(p_1.end(), tempVector.begin(), tempVector.end());
		grown_values.push_back(p_1);
	}

	//Add empty top and bottom slices
	{
		auto& topSlice = slices3d[slices3d.size() - 1];
		Eigen::Matrix3Xf top(3, topSlice.cols());
		for (int i = 0; i < topSlice.cols(); i++)
		{
			top.col(i) = topSlice.col(i) + Eigen::Vector3f({0, 0, static_cast<float>(z_distance)});
		}
		slices3d.push_back(top);

		auto& bottomSlice = slices3d[0];
		Eigen::Matrix3Xf bottom(3, bottomSlice.cols());
		for (int i = 0; i < bottomSlice.cols(); i++)
		{
			bottom.col(i) = bottomSlice.col(i) - Eigen::Vector3f({0, 0, static_cast<float>(z_distance)});
		}
		slices3d.insert(slices3d.begin(), bottom);
		//TODO: Inefficient O(n) call. Consider changing in all three locations.
	}

	{
		auto& topSlice = grownClusters[grownClusters.size() - 1];
		vector top(topSlice.size(), getClusterArray(newClusters + 1, newClusters));
		grownClusters.push_back(top);

		auto& bottomSlice = grownClusters[0];
		vector bottom(bottomSlice.size(), getClusterArray(newClusters + 1, newClusters));
		grownClusters.insert(grownClusters.begin(), bottom);
	}

	{
		auto& topSlice = grown_values[grown_values.size() - 1];
		vector top(topSlice.size(), getClusterArray(newClusters + 1, newClusters));
		grown_values.push_back(top);

		auto& bottomSlice = grown_values[0];
		vector bottom(bottomSlice.size(), getClusterArray(newClusters + 1, newClusters));
		grown_values.insert(grown_values.begin(), bottom);
	}

	//TODO: std::move for speed reasons
	tsv_return_type ret;
	ret.names = names;
	ret.slices = slices3d;
	ret.values = grown_values;
	ret.clusters = grownClusters;
	return ret;
}


Eigen::Vector2f getCentroid(colCoordMat sourceMatrix)
{
	return sourceMatrix.rowwise().mean();
}

colCoordMat translateToZeroCentroid(colCoordMat source_matrix)
{
	//Get the average of each row
	const auto centroid = getCentroid(source_matrix);

	//Subtract the centroid from each column;
	return source_matrix.colwise() - centroid;
}

Eigen::Matrix2f getSVDRotation(colCoordMat source_matrix, colCoordMat target_matrix)
{
	//Row 0 is x, row 1 is y

	//(* getting the centroid *)
	const colCoordMat zeroSource = translateToZeroCentroid(std::move(source_matrix));
	colCoordMat zeroTarget = translateToZeroCentroid(std::move(target_matrix));

	const Eigen::Matrix2f mat = zeroSource * zeroTarget.transpose();

	//(* SVD decomposition *)
	const Eigen::JacobiSVD<colCoordMat> svd(mat, Eigen::ComputeThinU | Eigen::ComputeThinV);
	//(* obtaining the rotation *)
	Eigen::Matrix2f r = (svd.matrixU() * svd.matrixV().transpose()).transpose(); //This is definitely a rotation matrix

	return r;
}

std::function<std::vector<coord>(std::vector<coord>)> getTransSVD(const std::vector<coord>& source,
                                                                  const std::vector<coord>& target)
{
	const auto sourceMatrix = vectorToMatrix(source);
	const auto targetMatrix = vectorToMatrix(target);

	//Convert to matrices
	const auto r = getSVDRotation(sourceMatrix, targetMatrix);
	const Eigen::Rotation2D<float> rotation(r);

	const Eigen::Vector2f sourceCentroid = getCentroid(sourceMatrix);
	const Eigen::Vector2f targetCentroid = getCentroid(targetMatrix);
	const Eigen::Vector2f targetCentroidTransform = targetCentroid - sourceCentroid;


	const Eigen::Translation2f netTranslation(targetCentroidTransform);
	const Eigen::Translation2f toZero(-1 * sourceCentroid);
	const Eigen::Translation2f fromZero(sourceCentroid);
	Eigen::Transform<float, 2, Eigen::Affine> finalTransform = netTranslation * fromZero * rotation * toZero;
	//Translate after rotate


	//(* transform *) Creating the function
	return {
		[finalTransform](std::vector<coord> points)
		{
			return matrixToVector(finalTransform * vectorToMatrix(std::move(points)));
		}
	};
}

std::vector<float> getClusterArray(size_t length, size_t i)
{
	std::vector<float> ret(length, 0);
	if (i < length)
	{
		ret[i] = 1;
	}
	return ret;
}
