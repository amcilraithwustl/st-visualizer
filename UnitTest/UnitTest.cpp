#include "pch.h"
#include "CppUnitTest.h"
#include "../st-visualizer/ImportFunctions.h"
#include <math.h>;
#include <sstream>;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace ImportTests
{
	TEST_CLASS(SVDTests)
	{
	public:
		TEST_METHOD(getSVDRotationTest)
		{
			//The main purpose to this set of tests is to make sure this works with cases of translation, and rotation

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

			int maxI = 1000;
			float pi = 3.1415926535;

			//Number of different matrixes to try
			for (int j = 0; j < 5; j++) {
				//Try 100 random tests with large length
				colMat m = colMat::Random(2, 20) * 100;
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

		TEST_METHOD(getTransSVDTest) {
			//The main purpose to this set of tests is to make sure this works with cases of scaling, translation, and rotation

			//Row coordinate matrix
			typedef Eigen::Matrix<float, Eigen::Dynamic, 2> rowMat;
			typedef Eigen::Matrix<float, 2, Eigen::Dynamic> colMat;

			auto i = 0;
			//Generic Rotation Testing Function
			auto genRotTest = [&i](std::vector<coord> source, std::vector<coord> target, std::vector<coord> finalSource, std::vector<coord> finalTest) {
				i++;
				
				auto func = getTransSVD(source, target);
				auto a = func(finalSource);
				std::stringstream failStream;
				failStream << i << std::endl << std::endl;
				std::string failString = failStream.str();
				std::wstring widestr = std::wstring(failString.begin(), failString.end());
				const wchar_t* widecstr = widestr.c_str();
				for (int j = 0; j < a.size(); j++) {
					Assert::AreEqual(a[i].first, finalTest[i].first);
					Assert::AreEqual(a[i].second, finalTest[i].second);
				}
			};

			int maxI = 1000;
			float pi = 3.1415926535;

			//Number of different matrixes to try
			for (int j = 0; j < 5; j++) {
				//Try 100 random tests with large length
				colMat m = colMat::Random(2,20) * 100;
				colMat m2 = colMat::Random(2, 20) * 100;
				for (int i = 0; i < maxI; i++) {
					auto rotation = Eigen::Rotation2Df(2 * pi * i / maxI).toRotationMatrix();
					Eigen::Transform<float, 2, Eigen::Affine> trans;
					trans.rotate(rotation);
					trans.translate(Eigen::Translation2f(100, 100).vector());
					genRotTest(
						matrixToVector(m),
						matrixToVector(trans * m), 
						matrixToVector(m2),
						matrixToVector(trans * m2)
					);
				}
			}
		}

	};
}
