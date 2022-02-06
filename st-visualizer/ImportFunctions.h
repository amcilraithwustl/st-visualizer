#pragma once
#include <string>
#include <vector>

typedef std::pair<int, int> coord;


std::vector<std::pair<std::vector<coord>, std::vector<coord>>> importAlignments(std::string alignmentFile);

void loadTSV(std::string tsvFile);