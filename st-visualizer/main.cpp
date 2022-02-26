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

	

	int maxI = 1000;

	//Number of different sets of coordinates to try

	for (int i = 0; i < maxI; i++) {
		//auto v1 = Eigen::Vector2f::Random();
		auto v2 = Eigen::Vector2f::Random();
		auto v1 = Eigen::Vector2f({ 1,0 });
		//auto v2 = Eigen::Vector2f({ 0,1 });
		auto origin = Eigen::Vector2f({ 0,0 });
		//Eigen::Vector2f hexCoord({ (rand() % 1000),(rand() % 1000) });
		Eigen::Vector2f hexCoord({ 1,1 });
		Eigen::Vector2f cartPoint = origin + (v1 * hexCoord(0)) + (v2 * hexCoord(1));
		std::cout << v2 << std::endl << std::endl;

		//Test their rotations
		auto testHexCoord = getCoords(cartPoint, origin, v1, v2);
		auto testCartPt = getPoint(hexCoord, origin, v1, v2);
		std::stringstream failStream;
		failStream << i << std::endl << std::endl
			<< cartPoint << std::endl << std::endl
			<< testCartPt << std::endl << std::endl
			<< hexCoord << std::endl << std::endl
			<< testHexCoord << std::endl << std::endl
			<< origin << std::endl << std::endl
			<< v1 << std::endl << std::endl
			<< v2 << std::endl << std::endl;
		std::string failString = failStream.str();
		std::wstring widestr = std::wstring(failString.begin(), failString.end());
		const wchar_t* widecstr = widestr.c_str();
		std::cout << failString << std::endl;
		//Assert::IsTrue(testHexCoord.isApprox(hexCoord), widecstr);
		//Assert::IsTrue(testCartPt.isApprox(cartPt), widecstr);
	}
	/*std::vector<std::string> arguments(argv, argv + argc);
	auto alignmentValues = importAlignments("../CRC112_transformation_pt_coord.csv");
	getTransSVD(alignmentValues[0].first, alignmentValues[0].second);
	loadTSV("../CRC_112C1_cell_type_coord_allspots.tsv", alignmentValues);*/
}