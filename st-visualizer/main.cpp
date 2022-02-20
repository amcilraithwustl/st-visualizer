#include <vector>
#include <functional>
#include "ImportFunctions.h"
#include <string>
#include "UtilityFunctions.h"
//https://wias-berlin.de/software/tetgen/
#include <iostream>
#include <Eigen/Dense>

int main(int argc, char** argv) {

	//Row coordinate matrix
	typedef Eigen::Matrix<float, Eigen::Dynamic, 2> rowMat;
	typedef Eigen::Matrix<float, 2, Eigen::Dynamic> colMat;

	auto i = 0;


	int maxI = 1000;
	float pi = 3.14159265358979323846264338327950288419716939937510;

	auto genRotTest = [&i](std::vector<coord> source, std::vector<coord> target, std::vector<coord> finalSource, std::vector<coord> finalTest) {
		i++;

		auto func = getTransSVD(source, target);
		auto a = func(finalSource);


		/*std::stringstream failStream;
		failStream << i << std::endl << std::endl;
		std::string failString = failStream.str();
		std::wstring widestr = std::wstring(failString.begin(), failString.end());
		const wchar_t* widecstr = widestr.c_str();*/

	};

	//Number of different matrixes to try
	for (int j = 0; j < 5; j++) {
		//Try 100 random tests with large length and a centroid of zero
		colMat m = colMat::Random(2, 10) * 100;
		colMat m2 = colMat::Random(2, 30) * 100;
		for (int i = 200; i < maxI; i++) {
			auto rotation = Eigen::Rotation2D<float>(2 * pi * i / maxI);
			auto translatoin = Eigen::Translation2f(1, 1);
			Eigen::Transform<float, 2, Eigen::Affine> trans = translatoin*rotation;
			genRotTest(
				matrixToVector(m),
				matrixToVector(trans * m),
				matrixToVector(m2),
				matrixToVector(trans * m2)
			);
		}
	}

	/*std::vector<std::string> arguments(argv, argv + argc);
	auto alignmentValues = importAlignments("../CRC112_transformation_pt_coord.csv");
	getTransSVD(alignmentValues[0].first, alignmentValues[0].second);
	loadTSV("../CRC_112C1_cell_type_coord_allspots.tsv", alignmentValues);*/
}