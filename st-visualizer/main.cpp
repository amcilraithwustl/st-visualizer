#include <vector>
#include <functional>
#include "ImportFunctions.h"
#include <string>
#include "UtilityFunctions.h"
//https://wias-berlin.de/software/tetgen/
#include <iostream>
#include <Eigen/Dense>

int main(int argc, char** argv) {
	typedef Eigen::Matrix<float, Eigen::Dynamic, 2> mat;

	//Generic Rotation Testing Function
	auto genRotTest = [](mat source, mat target) {
		// Send centroids to zero
		auto a = translateToZeroCentroid(source.transpose());
		auto b = translateToZeroCentroid(target.transpose());

		//Test their rotations
		auto rotation = getSVDRotation(a, b);

		std::cout << std::endl << "START" << std::endl << a
			<< std::endl << std::endl
			<< b << std::endl << std::endl << rotation * a << std::endl;
	};

	genRotTest(
		mat({ {1,1},{2,2} }),
		mat({ {1,-1},{2,-2} })
	);

	genRotTest(
		mat({ {1,1},{2,2}, {3,3},{4,4},{5,5} }),
		mat({ {-1,-1},{-2,-2}, {-3,-3},{-4,-4},{-5,-5} })
	);

	//Try 100 random tests with large length
	int maxI = 100;
	int pi = 3.1415926535;
	Eigen::Matrix<float, 2, Eigen::Dynamic> m = mat::Random(10, 2).transpose();
	for (int i = 0; i < maxI; i++) {
		auto rotation = Eigen::Rotation2D<float>(2 * pi * i / maxI).toRotationMatrix();
		genRotTest(
			m.transpose(),

			//Test random values against all radians possible
			(rotation * m).transpose()
		);
	}

	/*std::vector<std::string> arguments(argv, argv + argc);
	auto alignmentValues = importAlignments("../CRC112_transformation_pt_coord.csv");
	getTransSVD(alignmentValues[0].first, alignmentValues[0].second);
	loadTSV("../CRC_112C1_cell_type_coord_allspots.tsv", alignmentValues);*/
}