#include "pch.h"
#include "../st-visualizer/JSONParser.h"
#include "CppUnitTest.h"
#include "../st-visualizer/GrowAndCover.h"
#include <math.h>;
#include <fstream>;
#include <sstream>;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace ImportTests
{
	TEST_CLASS(HexTests)
	{
	public:
		TEST_METHOD(conversionTests)
		{
			auto i = 0;
			//Generic Rotation Testing Function
			auto genTest = [&i](Eigen::Vector2f cartPt, Eigen::Vector2f hexCoord, Eigen::Vector2f origin, Eigen::Vector2f v1, Eigen::Vector2f v2) {


				//Test their rotations
				auto testHexCoord = getCoords(cartPt, origin, v1, v2);
				auto testCartPt = getPoint(hexCoord, origin, v1, v2);
				std::stringstream failStream;
				failStream << i << std::endl
					<< cartPt << std::endl << std::endl
					<< testCartPt << std::endl << std::endl
					<< hexCoord << std::endl << std::endl
					<< testHexCoord << std::endl << std::endl
					<< origin << std::endl << std::endl
					<< v1 << std::endl << std::endl
					<< v2 << std::endl << std::endl;
				std::string failString = failStream.str();
				std::wstring widestr = std::wstring(failString.begin(), failString.end());
				const wchar_t* widecstr = widestr.c_str();
				Assert::IsTrue(hexCoord.isApprox(testHexCoord, 0.005), widecstr);
				Assert::IsTrue(testCartPt.isApprox(cartPt), widecstr);
				i++;

			};

			int maxI = 5000;

			//Number of different sets of coordinates to try

			for (int i = 0; i < maxI; i++) {
				//auto v1 = Eigen::Vector2f::Random();
				Eigen::Vector2f v1 = Eigen::Vector2f::Random();
				Eigen::Vector2f v2 = Eigen::Vector2f::Random();
				Eigen::Vector2f origin = Eigen::Vector2f::Random();
				Eigen::Vector2f hexCoord({ (rand() % 1000),(rand() % 1000) });
				Eigen::Vector2f cartPoint = origin + v1 * hexCoord(0) + v2 * hexCoord(1);

				genTest(cartPoint, hexCoord, origin, v1, v2);
			}

		}

		TEST_METHOD(getInliersTest)
		{
			//Generic Rotation Testing Function
			
			int maxI = 5;

			//Number of different sets of coordinates to try

			for (int j = 0; j < maxI; j++) {
				//Generate random hex coordinate system
				Eigen::Vector2f v1 = Eigen::Vector2f::Random();
				Eigen::Vector2f v2 = Eigen::Rotation2D(PI/3)*v1;
				Eigen::Vector2f origin = Eigen::Vector2f::Random();
				
				//Generate a random set of good coordinates
				Eigen::Matrix2Xi goodPoints = Eigen::Matrix2Xi::Random(2, 100);
				Eigen::Matrix2Xf badPoints = Eigen::Matrix2Xf::Zero(2, goodPoints.cols());
				//Generate a list to perturb
				std::vector < bool> editedLocations(false, goodPoints.cols());
				for (int i = 0; i < editedLocations.size(); i++) {
					auto intermediate = getPoint(goodPoints.col(i).cast<float>(), origin, v1, v2);
					if (rand() % 3 == 0) {
						//Make Wrong by between 0.3 and 0.7 of a unit in a random direction
						intermediate += Eigen::Vector2f::Random().normalized() * v1.norm() * ((rand() % 50)*0.01 + HEX_ROUNDING_ERROR);
						editedLocations[i] = true;
					}
					else {
						//Purturb slightly in a random direction
						intermediate += Eigen::Vector2f::Random().normalized() * v1.norm() * ((rand() % 10) * 0.1 * HEX_ROUNDING_ERROR);

					}
				}

				auto results = getInliers(badPoints, origin, v1);
				std::vector<bool> caughtLocations(false, editedLocations.size());
				for (auto a : results.first) {
					caughtLocations[a] = true;
				}

				int resLoc = 0;
				for (int i = 0; i < editedLocations.size(); i++) {
					//If that location should be bad, it shouldn't be included
					if (editedLocations[i]) {
						Assert::IsFalse(caughtLocations[i]);
					}
					else {
						Assert::IsTrue(caughtLocations[i]);
						Assert::IsTrue(results.second.col(resLoc).isApprox(goodPoints.col(i)));
						resLoc++;
					}

				}
			}

		}
	};
}