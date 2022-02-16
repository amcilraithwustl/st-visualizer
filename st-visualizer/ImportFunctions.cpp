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
using std::vector;
using std::string;

#define tissueConstant "1"

float getX(coord c) {
	return c.first;
}
float getY(coord c) {
	return c.second;
}

//From https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
std::vector<string> splitString(string s, string delimiter) {
	size_t pos_start = 0, pos_end, delim_len = delimiter.length();
	string token;
	std::vector<string> res;

	while ((pos_end = s.find(delimiter, pos_start)) != string::npos) {
		token = s.substr(pos_start, pos_end - pos_start);
		pos_start = pos_end + delim_len;
		res.push_back(token);
	}

	res.push_back(s.substr(pos_start));
	return res;

}

std::vector<coord> extractCoordinateSet(std::vector<float> top, std::vector<float> bottom) {
	if (top.size() != bottom.size()) throw("Size Mismatch");
	std::vector<coord> a;
	for (int i = 0; i < top.size(); i++) {
		a.push_back(coord(top[i], bottom[i]));
	}
	return a;
}


/// <summary>
/// Any impossible fields default to inf
/// </summary>
std::function<vector<float>(vector<string>)> convertRowToFloat([](vector<string> input) {
	return input << std::function<float(string)>([](string s) {
		try {
			return std::stof(s);
		}
		catch (...) {
			std::cerr << "WARNING: Invalid STOI attempt. Please check data: " << s << std::endl;
		}
		return std::numeric_limits<float>::infinity();
		}
	);
	}
);

std::vector<std::pair<std::vector<coord>, std::vector<coord>>> importAlignments(string alignmentFile)
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
	for (string row : lines) {
		vector<string> rowCells = splitString(row, ",");
		//Drop the first three columns
		csvCells.push_back(vector<string>(rowCells.begin() + 3, rowCells.end()));
	}
	//Drop the first row
	csvCells = vector<vector<string>>(csvCells.begin() + 1, csvCells.end());




	// Transform cells into ints
	auto transformCells = csvCells << convertRowToFloat;

	//Now we have n columns and m*4 rows of ints
	//This part transforms that into an m by 2 by n structure of coords

	//Transpose pairs
	vector<vector<coord>> coordinateSet;
	for (int i = 0; i < transformCells.size(); i += 2) {
		coordinateSet.push_back(extractCoordinateSet(transformCells[i], transformCells[i + 1]));
	}

	vector<std::pair<vector<coord>, vector<coord>>> finalSet;
	//line up in pairs 
	for (int i = 0; i < coordinateSet.size(); i += 2) {
		finalSet.push_back(std::pair<vector<coord>, vector<coord>>(coordinateSet[i], coordinateSet[i + 1]));
	}

	return finalSet;
}

bool canStof(std::string s) {
	try {
		auto a = std::stof(s);
		return true;
	}
	catch (...) {
		return false;
	}
}

void loadTSV(std::string tsvFile, vector<std::pair<vector<coord>, vector<coord>>> srctrgts)
{
	//Defining Constants
	//TODO: make this dynamic
	int clusterInd = 7;
	int minFeatureColumn = 8;
	int widBuffer = 2;
	int ransacNum = 8;
	unsigned int zDist = 60;

	vector<string> sliceNames({ "CRC_HT112C1_1", "CRC_HT112C1_2" });

	//Import raw file data
	std::ifstream aFile(tsvFile);
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
	std::function<vector<string>(string)> splitTabs = [](string s) {return splitString(s, "\t"); };
	auto rawData = lines << splitTabs;
	auto labels = rawData[0];
	auto nFeatures = labels.size() - 1;

	labels.erase(labels.begin() + 1, labels.begin() + minFeatureColumn); //Delete the useless labels

	auto bareData = vector<vector<string>>(rawData.begin() + 1, rawData.end());
	auto sliceIndex = 1;
	auto tissueIndex = 2;

	std::function<vector<vector<string>>(string)> transformSliceNames([&](string name) {
		std::function<bool(vector<string>)> isTissueSlice([&](vector<string> row) {
			return row[sliceIndex] == name && row[tissueIndex] == tissueConstant;
			});

		return filter(bareData, isTissueSlice);
		});

	auto records = sliceNames << transformSliceNames;
	std::pair<int, int> rowColInds({ 2,3 }); //TODO: Make Parameter and ensure this is exactly 2 length
	std::pair<int, int> xyInds({ rowColInds.second, rowColInds.first });
	vector<vector<coord>> slices; //Slice, Row, Coordinate
	for (int i = 0; i < sliceNames.size(); i++) {
		auto sliceCoordinates = records[i]
			<< convertRowToFloat //Row, Column, value
			<< std::function<coord(vector<float>)>([&](vector<float> v) {return coord({ v[xyInds.first], v[xyInds.second] }); });
		if (i == 0) {
			slices.push_back(sliceCoordinates);
		}
		else {
			auto adjustor = getTransSVD(srctrgts[i - 1].first, srctrgts[i-1].second); //TODO: make this do the transform
			slices.push_back(adjustor(sliceCoordinates));
		}
	}

	/*
	auto nclusters = filter(
		//Get the cluster index as a float
		rawData << convertRowToFloat << std::function<float(vector<float>)>([&](vector<float> v) {return v[clusterInd]; }),
		//We only want the ones with a 0 at that index
		std::function<bool(float)>([](float f) {return f == 0; })
	).size();//How many was that?

	auto clusters = records
		<< std::function<vector<vector<float>>(vector<vector<string>> v)>([&](vector<vector<string>> v) {
		return v
			<< convertRowToFloat
			<< std::function<vector<float>(vector<float>)>([&](vector<float> row) {
			if (row[tissueIndex] == 0) {
				return getClusterArray(nclusters + 1, nclusters);
			}
			else {
				return getClusterArray(nclusters + 1, row[clusterInd]);
			}
				});
			});
	auto vals = records
		<< std::function<vector<vector<float>>(vector<vector<string>> v)>([&](vector<vector<string>> slice) {
		return slice
			<< convertRowToFloat
			<< std::function<vector<float>(vector<float>)>([&](vector<float> row) {
			auto nfeatures = row.size() - minFeatureColumn;
			if (row[tissueIndex] == 0) {
				return getClusterArray(nfeatures + 1, nfeatures);
			}
			else {
				vector<float>features(row.begin() + minFeatureColumn, row.end());
				features.push_back(0);
				return features;
			}
				});
			});

	//add buffer to each slice and cover neighbors

	auto nslices = table(slices.size(), std::function<vector<coord>(size_t)>([&](size_t i) {
		auto boundingCoord = (i == 0)
			? slices[i + 1]
			: (i == slices.size() - 1
				? slices[i - 1]
				: concat(slices[i - 1], slices[i + 1]));

		return growAndCover(slices[i], boundingCoord, widBuffer, ransacNum);
		}));

	//Slices of sets of 3d coordinates
	auto horizBuffSlices = table(slices.size(), std::function<vector<coord3D>(size_t)>([&](size_t i) {
		return concat(slices[i], nslices[i]) << std::function<coord3D(coord)>([&](coord c) {
			return coord3D(getX(c), getY(c), i*zDist); //Each slice should be zDist apart, building upwards
			});
		}));

	//TODO: Add slices/clusters length check
	// One is cell type and the other is some type of marker (probably vals are markers, each number is the strength of a marker)(clusters are vector that takes the marker strength, and clustered all the dots based on the values of the vals (many less clusters) and give each dot a value based on the id
	auto arrayClusters = table(clusters.size(), std::function<void(size_t)>([&](size_t i) {
		auto sliceClusters = vector<vector<float>>(slices.size(), getClusterArray(nclusters + 1, nclusters));
		return concat(clusters[i], sliceClusters);
		}));

	auto arrayVals = table(vals.size(), std::function<void(size_t)>([&](size_t i) {
		auto sliceClusters = vector<vector<float>>(slices.size(), getClusterArray(nclusters + 1, nclusters));
		return concat(vals[i], sliceClusters);
		}));

	//Then buffer the top and bottom of both of them
	horizBuffSlices.push_back(horizBuffSlices[horizBuffSlices.size() - 1] << std::function<coord3D(coord3D)>([&](coord3D i) {return i + coord3D(0, 0,(float) zDist); }));
	horizBuffSlices.insert(horizBuffSlices.begin(), horizBuffSlices[0] << std::function<coord3D(coord3D)>([&](coord3D i) {return i + coord3D(0, 0, -1.0 *(float)zDist); }));
	//TODO: The final few lines of the mathematica
	*/

}


Eigen::Matrix<float, 2, Eigen::Dynamic> translateToZeroCentroid(Eigen::Matrix<float, 2, Eigen::Dynamic> sourceMatrix) {
	//Get the average of each row
	auto centroid = sourceMatrix.rowwise().mean();

	//Subtract the centroid from each column;
	return sourceMatrix.colwise() - centroid;
}
Eigen::Matrix<float, 2, 2> getSVDRotation(Eigen::Matrix<float, 2, Eigen::Dynamic> sourceMatrix, Eigen::Matrix<float, 2, Eigen::Dynamic> targetMatrix) {
	//Row 0 is x, row 1 is y
	//std::cout << sourceMatrix << std::endl << targetMatrix << std::endl;

	//(* getting the centroid *)
	Eigen::Matrix<float, 2, Eigen::Dynamic> zeroSource = translateToZeroCentroid(sourceMatrix);
	Eigen::Matrix<float, 2, Eigen::Dynamic> zeroTarget = translateToZeroCentroid(targetMatrix);
	auto mat = zeroSource * zeroTarget.transpose();

	//We are verified correct up to this point
	//std::cout << "MAT" << std::endl << mat << std::endl << std::endl;
	//(* SVD decomposition *)
	Eigen::JacobiSVD<Eigen::Matrix<float, 2, Eigen::Dynamic>> svd(mat, Eigen::ComputeThinU | Eigen::ComputeThinV);
	//std::cout << "VECTOR" << std::endl << svd.singularValues() << std::endl << std::endl;

	//std::cout << svd.matrixU() << std::endl << svd.matrixV() << std::endl;

	//(* obtaining the rotation *)
	Eigen::Matrix<float, 2, 2> r = (svd.matrixU()*svd.matrixV().transpose()).transpose(); //This is definitely a rotation matrix
	//std::cout << "ROTATION " << r << std::endl;

	return r;
}

std::function<std::vector<coord>(std::vector<coord>)> getTransSVD(const std::vector<coord>& source, const std::vector<coord>& target)
{
	//Convert to matrixes

	auto vectorToMatrix = [&](std::vector<coord> source) {
		//Row 0 is x, row 1 is y
		Eigen::Matrix<float, 2, Eigen::Dynamic> sourceMatrix(2, source.size());
		for (int i = 0; i < source.size(); i++) {
			sourceMatrix(0, i) = source[i].first;
			sourceMatrix(1, i) = source[i].second;
		}
		return sourceMatrix;
	};

	auto matrixToVector = [&](Eigen::Matrix<float, 2, Eigen::Dynamic> sourceMatrix) {
		//Row 0 is x, row 1 is y
		std::vector<coord> source(sourceMatrix.cols(), coord());
		for (int i = 0; i < sourceMatrix.cols(); i++) {
			source[i].first = sourceMatrix(0, i);
			source[i].second = sourceMatrix(1, i);
		}
		return source;
	};

	auto r = getSVDRotation(vectorToMatrix(source), vectorToMatrix(target));

	//(* transform *) Creating the function
	return std::function<std::vector<coord>(std::vector<coord>)>([&](std::vector<coord> points) {
		return matrixToVector(r * vectorToMatrix(points));
		});
}

std::vector<float> getClusterArray(size_t length, size_t i)
{
	std::vector<float>ret(length, 0);
	if (i < length) {
		ret[i] = 1;
	}
	return ret;
}
