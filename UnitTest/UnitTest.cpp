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
		TEST_METHOD(SVDRotationIdentities) {
			typedef Eigen::Matrix<float, 2, Eigen::Dynamic> mat;

			//Checking non-rotated matrix legality
			Assert::IsTrue(getSVDRotation(
				mat({ {1,2,0},{0,3,0} }),
				mat({ {1,2,0},{0,3,0} })
			).isApprox(
				Eigen::Matrix<float, 2, 2>({ {1,0}, {0,1} })
			));

			Assert::IsTrue(getSVDRotation(
				mat({ {3,4,2},{0,3,0} }),
				mat({ {1,2,0},{0,3,0} })
			).isApprox(
				Eigen::Matrix<float, 2, 2>({ {1,0}, {0,1} })
			));

			Assert::IsTrue(getSVDRotation(
				mat({ {1,2,0},{1,4,1} }),
				mat({ {1,2,0},{0,3,0} })
			).isApprox(
				Eigen::Matrix<float, 2, 2>({ {1,0}, {0,1} })
			));

			Assert::IsTrue(getSVDRotation(
				mat({ {1,2,0,0},{0,3,0,1} }),
				mat({ {1,2,0,0},{0,3,0,1} })
			).isApprox(
				Eigen::Matrix<float, 2, 2>({ {1,0}, {0,1} })
			));
		}
		TEST_METHOD(SVDRotationMathematicaTests)
		{
			//The main purpose to this set of tests is to make sure this works with cases of scaling, translation, and rotation

			//Row coordinate matrix
			typedef Eigen::Matrix<float, Eigen::Dynamic, 2> mat;

			auto i = 0;
			//Generic Rotation Testing Function
			auto genRotTest = [&i](mat source, mat target) {
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
				mat({ {1,1},{2,2} }),
				mat({ {1,-1},{2,-2} })
			);

			genRotTest(
				mat({ {1,1},{2,2}, {3,3},{4,4},{5,5} }),
				mat({ {-1,-1},{-2,-2}, {-3,-3},{-4,-4},{-5,-5} })
			);

			//Try 100 random tests with large length
			int maxI = 1000;
			float pi = 3.1415926535;
			Eigen::Matrix<float, 2, Eigen::Dynamic> m = mat::Random(3, 2).transpose() * 100;
			for (int i = 0; i < maxI; i++) {
				auto rotation = Eigen::Rotation2D<float>(2 * pi * i / maxI).toRotationMatrix();
				genRotTest(
					m.transpose(),

					//Test random values against all radians possible within range
					(rotation * m).transpose()
				);
			}

		}

	};
}
