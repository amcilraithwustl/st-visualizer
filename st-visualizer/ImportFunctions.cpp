#include "ImportFunctions.h"
#include "UtilityFunctions.h"
#include <fstream>
#include <iostream>
#include <strstream>
#include <functional>
#include <iostream>
using std::vector;
using std::string;
#define tissueConstant "1"

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

std::vector<coord> extractCoordinateSet(std::vector<int> top, std::vector<int> bottom) {
	if (top.size() != bottom.size()) throw("Size Mismatch");
	std::vector<coord> a;
	for (int i = 0; i < top.size(); i++) {
		a.push_back(coord(top[i], bottom[i]));
	}
	return a;
}

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


	std::function<vector<int>(vector<string>)> convertRowToInt([](vector<string> input) {
		return input << std::function<int(string)>(
			[](string s) {
				try {
					return std::stoi(s);
				}
				catch (...) {
					std::cerr << "WARNING: Invalid STOI attempt. Please check data: " << s << std::endl;
				}
				return 0;
			}
		);
		}
	);

	// Transform cells into ints
	auto transformCells = csvCells << convertRowToInt;

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

void loadTSV(std::string tsvFile)
{
	//Defining Constants
	//TODO: make this dynamic
	int minFeatureColumn = 8;
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

}
