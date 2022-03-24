#include <vector>
#include "ImportFunctions.h"
#include <string>
#include "JSONParser.h"
//https://wias-berlin.de/software/tetgen/




int main(int argc, char** argv) {
	std::vector<std::string> arguments(argv, argv + argc);
	auto alignmentValues = importAlignments("../CRC112_transformation_pt_coord.csv");
	getTransSVD(alignmentValues[0].first, alignmentValues[0].second);
	// loadTsv("../CRC_112C1_cell_type_coord_allspots.tsv", alignmentValues);
}