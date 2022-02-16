#include "pch.h"
#include "CppUnitTest.h"
#include "../st-visualizer/ImportFunctions.h"
#include <math.h>;
#include <sstream>;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace UnitTest
{
	TEST_CLASS(SVDTests)
	{
	public:
		TEST_METHOD(SVDRotationGeneratedTests)
		{
			//The main purpose to this set of tests is to make sure this works with cases of scaling, translation, and rotation

			//Row coordinate matrix
			typedef Eigen::Matrix<float, Eigen::Dynamic, 2> rowMat;
			typedef Eigen::Matrix<float, 2, Eigen::Dynamic> colMat;

			auto i = 0;
			//Generic Rotation Testing Function
			auto genRotTest = [&i](rowMat source, rowMat target) {
				i++;
				// Send centroids to zero
				auto a = translateToZeroCentroid(source.transpose());
				auto b = translateToZeroCentroid(target.transpose());

				//Test their rotations
				auto rotation = getSVDRotation(a, b);
				std::stringstream failStream;
				failStream << i << std::endl<< a << std::endl << std::endl << b << std::endl << std::endl << rotation << std::endl << std::endl << rotation * a;;
				std::string failString = failStream.str();
				std::wstring widestr = std::wstring(failString.begin(), failString.end());
				const wchar_t* widecstr = widestr.c_str();
				Assert::IsTrue((rotation * a).isApprox(b), widecstr);
			};

			genRotTest(
				rowMat({ {1,1},{2,2} }),
				rowMat({ {1,-1},{2,-2} })
			);

			genRotTest(
				rowMat({ {1,1},{2,2}, {3,3},{4,4},{5,5} }),
				rowMat({ {-1,-1},{-2,-2}, {-3,-3},{-4,-4},{-5,-5} })
			);

			int maxI = 1000;
			float pi = 3.1415926535;

			//Number of different matrixes to try
			for (int j = 0; j < 5; j++) {
				//Try 100 random tests with large length
				colMat m = rowMat::Random(20, 2).transpose() * 100;
				for (int i = 0; i < maxI; i++) {
					auto rotation = Eigen::Rotation2D<float>(2 * pi * i / maxI).toRotationMatrix();
					genRotTest(
						m.transpose(),

						//Test random values against all radians possible within range
						(rotation * m).transpose()
					);
				}
			}
		}

	};
}
