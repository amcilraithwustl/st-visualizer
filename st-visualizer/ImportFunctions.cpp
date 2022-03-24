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

#define TISSUE_CONSTANT "1"

float get(coord c)
{
	return c.first;
}

float getY(coord c)
{
	return c.second;
}

//From https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
std::vector<string> splitString(const string& s, const string& delimiter)
{
	size_t pos_start = 0, pos_end, delim_len = delimiter.length();
	string token;
	std::vector<string> res;

	while ((pos_end = s.find(delimiter, pos_start)) != string::npos)
	{
		token = s.substr(pos_start, pos_end - pos_start);
		pos_start = pos_end + delim_len;
		res.push_back(token);
	}

	res.push_back(s.substr(pos_start));
	return res;
}

std::vector<coord> extractCoordinateSet(std::vector<float> top, std::vector<float> bottom)
{
	if (top.size() != bottom.size()) throw "Size Mismatch";
	std::vector<coord> a;
	a.reserve(top.size());
	for (size_t i = 0; i < top.size(); i++)
	{
		a.emplace_back(coord(top[i], bottom[i]));
	}
	return a;
}




std::vector<std::pair<std::vector<coord>, std::vector<coord>>> importAlignments(const string& alignmentFile)
{
	//Import raw file data
	std::ifstream aFile(alignmentFile);
	string line;
	std::vector<string> lines;
	if (aFile.is_open())
	{
		while (std::getline(aFile, line))
		{
			lines.push_back(line);
		}
		aFile.close();
	}

	//Split into cells
	vector<vector<string>> csvCells;
	for (string row : lines)
	{
		vector<string> rowCells = splitString(row, ",");
		//Drop the first three columns
		csvCells.push_back(vector(rowCells.begin() + 3, rowCells.end()));
	}
	//Drop the first row
	csvCells = vector(csvCells.begin() + 1, csvCells.end());

	/// Any impossible fields default to inf
	std::function<vector<float>(vector<string>)> convertRowToFloat([](const vector<string>& input)
		{
			return input << std::function<float(string)>([](const string& s)
				{
					try
					{
						return std::stof(s);
					}
					catch (...)
					{
						std::cerr << "WARNING: Invalid STOI attempt. Please check data: " << s << std::endl;
					}
					return std::numeric_limits<float>::infinity();
				}
			);
		}
	);

	// Transform cells into ints
	auto transformCells = csvCells << convertRowToFloat;

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
		finalSet.push_back(std::pair(coordinateSet[i], coordinateSet[i + 1]));
	}

	return finalSet;
}

tsvReturnType loadTsv(std::string fname, std::vector<std::string> sliceNames, unsigned int sliceInd,
                      unsigned int tissueInd,
                      std::pair<
	                      unsigned, unsigned> rowcolInds, unsigned int clusterInd,
                      std::vector<unsigned> featureInds, unsigned int zDis,
                      std::vector<std::pair<std::vector<coord>, std::vector<coord>>> srctgts)
{
	//Import raw file data
	std::ifstream aFile(fname);
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
	for (auto index : featureInds)
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

	unsigned int nclusters = max + 1;
	auto nfeatures = featureInds.size();

	vector<vector<vector<string>>> records;
	for (auto name : sliceNames)
	{
		vector<vector<string>> temp;
		for (auto row : tab)
		{
			if (row[sliceInd] == name && row[clusterInd] == "1")
			{
				temp.push_back(row);
			}
		}
		records.push_back(temp);
	}

	vector<Eigen::Matrix2Xf> slices;
	std::pair xyInds(rowcolInds.second, rowcolInds.first);
	for (size_t i = 0; i < records.size(); i++)
	{
		auto& record = records[i];

		vector<std::pair<float, float>> temp;
		for (auto& row : record)
		{
			std::pair point(std::stof(row[xyInds.first]), std::stof(row[xyInds.second]));
			temp.push_back(point);
		}
		if (i == 0)
		{
			slices.push_back(vectorToMatrix(temp));
		}
		else
		{
			std::function transform = getTransSVD(srctgts[i - 1].first, srctgts[i - 1].second);
			slices.push_back(vectorToMatrix(transform(temp)));
		}
	}

	vector<vector<vector<float>>> clusters;
	for (const auto& record : records)
	{
		vector<vector<float>> temp;
		for (const auto& row : record)
		{
			if (row[tissueInd] == "0")
			{
				temp.push_back(getClusterArray(nclusters + 1, nclusters + 1));
			}
			else
			{
				temp.push_back(getClusterArray(nclusters + 1, std::stoi(row[clusterInd]) + 1));
			}
		}
		clusters.push_back(temp);
	}

	vector<vector<vector<float>>> vals;
	for (const auto& record : records)
	{
		vector<vector<float>> temp;
		for (const auto& row : record)
		{
			if (row[tissueInd] == "0")
			{
				temp.push_back(getClusterArray(nfeatures + 1, nfeatures + 1));
			}
			else
			{
				vector<float> a;
				for (auto ind : featureInds)
				{
					a.push_back(std::stof(row[ind]));
				}
				a.emplace_back(0);
				temp.push_back(a);
			}
		}
		vals.push_back(temp);
	}

	//Add buffer to each slice and grow and cover neighboring slices
	vector<Eigen::Matrix2Xf> nslices; //Holds the points that we are growing into
	nslices.reserve(slices.size() + 2);
	const auto widBuffer = 2;
	const auto numRANSAC = 5;
	for (size_t i = 0; i < slices.size(); i++)
	{
		Eigen::Matrix2Xf result;
		if (i == 0)
		{
			result = growAndCover(slices[i], slices[i + 1], widBuffer, numRANSAC);
		}
		else if (i == slices.size() - 1)
		{
			result = growAndCover(slices[i], slices[i - 1], widBuffer, numRANSAC);
		}
		else
		{
			Eigen::Matrix2Xf temp(2, slices[i + 1].cols() + slices[i - 1].cols());
			temp << slices[i + 1], slices[i - 1];
			result = growAndCover(slices[i], temp, widBuffer, numRANSAC);
		}
		nslices.push_back(result);
	}

	vector<Eigen::Matrix3Xf> slices3d;
	slices3d.reserve(nslices.size());
	for (size_t i = 0; i < nslices.size(); i++)
	{
		Eigen::Matrix2Xf layer2d(2, nslices[i].cols() + slices[i].cols());
		layer2d << nslices[i], slices[i]; //The new and old points on that layer
		Eigen::Matrix3Xf layer3d(3, layer2d.cols());
		for (int j = 0; j < layer2d.cols(); j++)
		{
			layer3d.col(j)(0) = layer2d.col(j)(0);
			layer3d.col(j)(1) = layer2d.col(j)(1);
			layer3d.col(j)(2) = static_cast<float>(zDis * i);
		}
		slices3d.push_back(layer3d);
	}

	//Associating data with the new points on their respective slices
	vector<vector<vector<float>>> grownClusters;
	for (size_t i = 0; i < clusters.size(); i++)
	{
		auto p_1 = clusters[i];
		auto p_2 = nslices[i];

		vector tempVector(p_2.cols(), getClusterArray(nclusters + 1, nclusters));

		p_1.insert(p_1.end(), tempVector.begin(), tempVector.end());
		grownClusters.push_back(p_1);
	}

	vector<vector<vector<float>>> grown_vals;
	for (size_t i = 0; i < vals.size(); i++)
	{
		auto p_1 = vals[i];
		auto p_2 = nslices[i];

		vector tempVector(p_2.cols(), getClusterArray(nfeatures + 1, nfeatures));

		p_1.insert(p_1.end(), tempVector.begin(), tempVector.end());
		grown_vals.push_back(p_1);
	}

	//Add empty top and bottom slices
	{
		auto& topSlice = slices3d[slices3d.size() - 1];
		Eigen::Matrix3Xf top(3, topSlice.cols());
		for (int i = 0; i < topSlice.cols(); i++)
		{
			top.col(i) = topSlice.col(i) + Eigen::Vector3f({0, 0, static_cast<float>(zDis)});
		}
		slices3d.push_back(top);

		auto& bottomSlice = slices3d[0];
		Eigen::Matrix3Xf bottom(3, bottomSlice.cols());
		for (int i = 0; i < bottomSlice.cols(); i++)
		{
			bottom.col(i) = bottomSlice.col(i) - Eigen::Vector3f({0, 0, static_cast<float>(zDis)});
		}
		slices3d.insert(slices3d.begin(), bottom);
		//TODO: Innefficiant O(n) call. Consider changing in all three locations.
	}

	{
		auto& topSlice = grownClusters[grownClusters.size() - 1];
		vector top(topSlice.size(), getClusterArray(nclusters + 1, nclusters));
		grownClusters.push_back(top);

		auto& bottomSlice = grownClusters[0];
		vector bottom(bottomSlice.size(), getClusterArray(nclusters + 1, nclusters));
		grownClusters.insert(grownClusters.begin(), bottom);
	}

	{
		auto& topSlice = grown_vals[grown_vals.size() - 1];
		vector top(topSlice.size(), getClusterArray(nclusters + 1, nclusters));
		grown_vals.push_back(top);

		auto& bottomSlice = grown_vals[0];
		vector bottom(bottomSlice.size(), getClusterArray(nclusters + 1, nclusters));
		grown_vals.insert(grown_vals.begin(), bottom);
	}

	//TODO: std::move for speed reasons
	tsvReturnType ret;
	ret.names = names;
	ret.slices = slices3d;
	ret.vals = grown_vals;
	ret.clusters = grownClusters;
	return ret;
}


Eigen::Vector2f getCentroid(colCoordMat sourceMatrix)
{
	return sourceMatrix.rowwise().mean();
}

colCoordMat translateToZeroCentroid(colCoordMat sourceMatrix)
{
	//Get the average of each row
	auto centroid = getCentroid(sourceMatrix);

	//Subtract the centroid from each column;
	return sourceMatrix.colwise() - centroid;
}

Eigen::Matrix2f getSVDRotation(colCoordMat sourceMatrix, colCoordMat targetMatrix)
{
	//Row 0 is x, row 1 is y

	//(* getting the centroid *)
	colCoordMat zeroSource = translateToZeroCentroid(std::move(sourceMatrix));
	colCoordMat zeroTarget = translateToZeroCentroid(std::move(targetMatrix));

	Eigen::Matrix2f mat = zeroSource * zeroTarget.transpose();

	//(* SVD decomposition *)
	Eigen::JacobiSVD<colCoordMat> svd(mat, Eigen::ComputeThinU | Eigen::ComputeThinV);
	//(* obtaining the rotation *)
	Eigen::Matrix2f r = (svd.matrixU() * svd.matrixV().transpose()).transpose(); //This is definitely a rotation matrix

	return r;
}

std::function<std::vector<coord>(std::vector<coord>)> getTransSVD(const std::vector<coord>& source,
                                                                  const std::vector<coord>& target)
{
	auto sourceMatrix = vectorToMatrix(source);
	auto targetMatrix = vectorToMatrix(target);

	//Convert to matrixes
	auto r = getSVDRotation(sourceMatrix, targetMatrix);
	Eigen::Rotation2D<float> rotation(r);

	Eigen::Vector2f sourceCentroid = getCentroid(sourceMatrix);
	Eigen::Vector2f targetCentroid = getCentroid(targetMatrix);
	Eigen::Vector2f targetCentroidTransform = targetCentroid - sourceCentroid;


	Eigen::Translation2f netTranslation(targetCentroidTransform);
	Eigen::Translation2f toZero(-1 * sourceCentroid);
	Eigen::Translation2f fromZero(sourceCentroid);
	Eigen::Transform<float, 2, Eigen::Affine> finalTransform = netTranslation * fromZero * rotation * toZero;
	//Translate after rotate


	//(* transform *) Creating the function
	return std::function([finalTransform](std::vector<coord> points)
	{
		return matrixToVector(finalTransform * vectorToMatrix(std::move(points)));
	});
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
