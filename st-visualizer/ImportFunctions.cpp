#include "ImportFunctions.h"
#include <fstream>
#include <strstream>
#include <functional>
#include <iostream>


//From https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
std::vector<std::string> splitString(std::string s, std::string delimiter) {
		size_t pos_start = 0, pos_end, delim_len = delimiter.length();
		std::string token;
		std::vector<std::string> res;

		while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
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

std::vector<std::pair<std::vector<coord>, std::vector<coord>>> importAlignments(std::string alignmentFile)
{
	std::ifstream aFile(alignmentFile);
	std::string line;
	std::vector<std::string> lines;
	if (aFile.is_open())
	{
		while (std::getline(aFile, line))
		{
			lines.push_back(line);
		}
		aFile.close();
	}

	std::vector<std::vector<std::string>> csvCells;
	for (std::string row : lines) {
		std::vector<std::string> rowCells = splitString(row, ",");
		//Drop the first three columns
		csvCells.push_back(std::vector<std::string>(rowCells.begin()+3, rowCells.end()));
	}
	//Drop the first row
	csvCells = std::vector<std::vector<std::string>>(csvCells.begin() + 1, csvCells.end());

	//Turn into ints
	std::vector<std::vector<int>> transformCells;
	for (auto& row : csvCells) {
		transformCells.push_back({});
		for (auto& cell : row) {
			transformCells[transformCells.size() - 1].push_back(cell.size() ? std::stoi(cell) : 0);
		}
	}


	//Now we have n columns and m*4 rows of ints
	//This part transforms that into an m by 2 by n structure of coords

	//Transpose pairs
	std::vector<std::vector<coord>> coordinateSet;
	for (int i = 0; i < transformCells.size(); i += 2) {
		coordinateSet.push_back(extractCoordinateSet(transformCells[i], transformCells[i + 1]));
	}

	std::vector< std::pair<std::vector<coord>, std::vector<coord>>> finalSet;
	//line up in pairs 
	for (int i = 0; i < coordinateSet.size(); i += 2) {
		finalSet.push_back(std::pair<std::vector<coord>, std::vector<coord>>(coordinateSet[i], coordinateSet[i + 1]));
	}
	
	return finalSet;
}