#include <vector>
#include <functional>
#include "ImportFunctions.h"
#include <string>
#include "UtilityFunctions.h"
#include "GrowAndCover.h"
#include <fstream>
#include "JSONParser.h"
//https://wias-berlin.de/software/tetgen/
#include <iostream>
#include <Eigen/Dense>



int main(int argc, char** argv) {
	Eigen::Matrix2Xi a;
	a.conservativeResize(Eigen::NoChange, a.cols()+1);
	a.col(a.cols() - 1) = Eigen::Vector2i({ 1,1 });
	std::cout << a << std::endl;
	/*std::vector<std::string> arguments(argv, argv + argc);
	auto alignmentValues = importAlignments("../CRC112_transformation_pt_coord.csv");
	getTransSVD(alignmentValues[0].first, alignmentValues[0].second);
	loadTSV("../CRC_112C1_cell_type_coord_allspots.tsv", alignmentValues);*/
}