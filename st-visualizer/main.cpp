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
	using json = nlohmann::json;
	int i = 0;

	auto jsonToMatrix = [](const json& source) {
		//This should only be used for testing. Not logic safe.

		Eigen::Matrix2Xf a(2, source.size());

		for (int i = 0; i < source.size(); i++) {
			a(0, i) = source[i][0];
			a(1, i) = source[i][1];
		}
		return a;
	};
	// read a JSON file
	std::ifstream file("C:\\Users\\Aiden McIlraith\\Documents\\GitHub\\st-visualizer\\growTests.json");
	auto a = file.is_open();
	json j = json::parse(file);
	json ret = json::array();
	for (json& test : j) {
		Eigen::Matrix2Xf pts = jsonToMatrix(test);
		auto results = getGridAndCoords(pts, 5);
		json resJson({ json(test), json(results.first), json(matrixToVector(results.second.cast<float>()))});
		ret.push_back(resJson);
	}

	std::ofstream f("C:\\Users\\Aiden McIlraith\\Documents\\GitHub\\st-visualizer\\hexResults.json");
	f << ret;
	
	/*std::vector<std::string> arguments(argv, argv + argc);
	auto alignmentValues = importAlignments("../CRC112_transformation_pt_coord.csv");
	getTransSVD(alignmentValues[0].first, alignmentValues[0].second);
	loadTSV("../CRC_112C1_cell_type_coord_allspots.tsv", alignmentValues);*/
}