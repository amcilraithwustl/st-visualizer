#include <fstream>
#include <iostream>
#include <vector>
#include "ImportFunctions.h"
#include <string>

#include "Contour2D.h"
#include "JSONParser.h"
using namespace nlohmann;
//https://wias-berlin.de/software/tetgen/


int main(int argc, char** argv)
{
	const auto alignmentValues = importAlignments("C:/Users/Aiden McIlraith/Documents/GitHub/st-visualizer/NMK_F_transformation_pt_coord.csv");
	const auto results = loadTsv(
		"C:/Users/Aiden McIlraith/Documents/GitHub/st-visualizer/NMK_20201201_cell_type_coord_allspots.tsv",
		std::vector<std::string>({ "NMK_F_U1","NMK_F_U2","NMK_F_U3","NMK_F_U4" }),
		1,
		2,
		std::pair<unsigned, unsigned>(3, 4),
		5,
		std::vector<unsigned>({ 6,7,8,9, 10, 11, 12, 13, 14, 15 }),
		60,
		alignmentValues
	);

	getSectionContoursAll(results.slices, results.values, 1);

}
