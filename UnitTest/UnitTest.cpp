#include "pch.h"
#include "CppUnitTest.h"
#include "../st-visualizer/ImportFunctions.h"
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

		}
	};
}
