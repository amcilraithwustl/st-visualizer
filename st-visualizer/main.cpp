#include <vector>
#include <functional>
#include "ImportFunctions.h"
#include <string>
#include "UtilityFunctions.h"
#include <fstream>
#include "JSONParser.h"
//https://wias-berlin.de/software/tetgen/
#include <iostream>
#include <Eigen/Dense>



int main(int argc, char** argv) {
	{
		using json = nlohmann::json;
		int i = 0;
		auto genRotTest = [&i](std::vector<coord> source, std::vector<coord> target, std::vector<coord> finalSource, std::vector<coord> finalTest) {
			i++;

			auto func = getTransSVD(source, target);
			auto a = vectorToMatrix(func(finalSource));


			std::stringstream failStream;
			failStream << i << std::endl << a << std::endl << std::endl << vectorToMatrix(finalTest) << std::endl << std::endl << vectorToMatrix(source) << std::endl << std::endl << vectorToMatrix(target);

			std::string failString = failStream.str();
			std::wstring widestr = std::wstring(failString.begin(), failString.end());
			const wchar_t* widecstr = widestr.c_str();

			if (!a.isApprox(vectorToMatrix(finalTest))) {
				std::cout << "FIRST" << std::endl << a - vectorToMatrix(finalTest) << std::endl << std::endl;
				std::cout << "Result" << std::endl << vectorToMatrix(finalSource).rowwise().mean() << std::endl << std::endl;
				std::cout << "Test" << std::endl << vectorToMatrix(finalTest).rowwise().mean() << std::endl << std::endl;
				
			}

		};
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
		std::ifstream file("../UnitTest/svd.json");
		auto a = file.is_open();
		json j = json::parse(file);

		for (json& test : j) {
			auto alignSource = jsonToMatrix(test[0]);
			auto alignTarget = jsonToMatrix(test[1]);
			auto mapSource = jsonToMatrix(test[2]);
			auto mapTarget = jsonToMatrix(test[3]);
			genRotTest(
				matrixToVector(alignSource),
				matrixToVector(alignTarget),
				matrixToVector(mapSource),
				matrixToVector(mapTarget)
			);
		}

	}
	/*std::vector<std::string> arguments(argv, argv + argc);
	auto alignmentValues = importAlignments("../CRC112_transformation_pt_coord.csv");
	getTransSVD(alignmentValues[0].first, alignmentValues[0].second);
	loadTSV("../CRC_112C1_cell_type_coord_allspots.tsv", alignmentValues);*/
}