#include <vector>
#include <functional>
#include "ImportFunctions.h"
#include <string>
#include "UtilityFunctions.h"

//https://wias-berlin.de/software/tetgen/



int main(int argc, char** argv) {
	std::vector<std::string> arguments(argv, argv + argc);
	auto alignmentValues = importAlignments("../CRC112_transformation_pt_coord.csv");
	loadTSV("../CRC_112C1_cell_type_coord_allspots.tsv");
}