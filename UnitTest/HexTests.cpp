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

			int maxI = 5000;

			//Number of different sets of coordinates to try

			for (int j = 0; j < maxI; j++) {
				//Generate random hex coordinate system
				Eigen::Vector2f v1 = Eigen::Vector2f::Random() * 10;
				Eigen::Vector2f v2 = Eigen::Rotation2D(PI / 3) * v1;
				Eigen::Vector2f origin = Eigen::Vector2f::Random();

				//Generate a random set of good coordinates
				Eigen::Matrix2Xi goodPoints = Eigen::Matrix2Xi::Random(2, 100);
				Eigen::Matrix2Xf badPoints = Eigen::Matrix2Xf::Zero(2, goodPoints.cols());
				//Generate a list to perturb
				std::vector < bool> editedLocations(goodPoints.cols(), false);
				for (int i = 0; i < editedLocations.size(); i++) {
					auto intermediate = getPoint(goodPoints.col(i).cast<float>(), origin, v1, v2);
					if (rand() % 3 == 0) {
						//Make Wrong by up to twice the rounding error
						float errorMargin = HEX_ROUNDING_ERROR + ((rand() % 9) + 1) * 0.1 * HEX_ROUNDING_ERROR;
						Eigen::Vector2f change = Eigen::Vector2f::Random().normalized() * v1.norm() * errorMargin;
						intermediate += change;
						editedLocations[i] = true;
					}
					else {
						//Purturb slightly in a random direction
						intermediate += Eigen::Vector2f::Random().normalized() * v1.norm() * ((rand() % 10) * 0.1 * HEX_ROUNDING_ERROR);

					}
					badPoints.col(i) = intermediate;
				}

				auto results = getInliers(badPoints, origin, v1);
				std::vector<bool> caughtLocations(editedLocations.size(), false);
				for (auto a : results.first) {
					caughtLocations[a] = true;
				}

				int resLoc = 0;
				for (int i = 0; i < editedLocations.size(); i++) {
					if (editedLocations[i]) {
						//If that location should be bad, it shouldn't be included
						Assert::IsTrue(!caughtLocations[i]);
					}
					else {
						//If the point is there, make sure it should be there and that it was inferred correctly
						Assert::IsTrue(caughtLocations[i]);
						Assert::IsTrue(results.second.col(resLoc).isApprox(goodPoints.col(i)));
						resLoc++;
					}
				}
				//Make sure we checked all of the returned points
				Assert::IsTrue(resLoc == results.second.cols());
			}

		}

		TEST_METHOD(initGridInliersTest)
		{
			//Generic Rotation Testing Function

			int maxI = 50;

			//Number of different sets of coordinates to try

			for (int j = 0; j < maxI; j++) {
				//Generate random hex coordinate system
				Eigen::Vector2f v1 = Eigen::Vector2f::Random().normalized();
				Eigen::Vector2f v2 = Eigen::Rotation2D(PI / 3) * v1;
				Eigen::Vector2f origin = Eigen::Vector2f::Random();

				//Generate a random set of good coordinates
				int gridSize = 4;

				//The grid should be full
				Eigen::Matrix2Xi goodPoints = Eigen::Matrix2Xi::Zero(2, gridSize * gridSize);
				for (int i = 0; i < gridSize * gridSize; i++) {
					goodPoints.col(i) = Eigen::Vector2i({i % gridSize, i/gridSize});
				}
				Eigen::Matrix2Xf badPoints = Eigen::Matrix2Xf::Zero(2, goodPoints.cols());
				//Generate a list to perturb
				std::vector < bool> editedLocations(goodPoints.cols(), false);
				for (int i = 0; i < editedLocations.size(); i++) {
					auto intermediate = getPoint(goodPoints.col(i).cast<float>(), origin, v1, v2);
					//if (rand() % 3 == -1) {
					//	//Make Wrong by up to twice the rounding error
					//	float errorMargin = HEX_ROUNDING_ERROR + ((rand() % 9) + 1) * 0.1 * HEX_ROUNDING_ERROR;
					//	Eigen::Vector2f change = Eigen::Vector2f::Random().normalized() * v1.norm() * errorMargin;
					//	intermediate += change;
					//	editedLocations[i] = true;
					//}
					//else {
					//	//Purturb slightly in a random direction
					//	intermediate += Eigen::Vector2f::Random().normalized() * v1.norm() * ((rand() % 10) * 0.1 * HEX_ROUNDING_ERROR);

					//}
					badPoints.col(i) = intermediate;
				}

				auto results = initGridInliers(badPoints, 50);
				auto calcOrigin = results.first.first;
				auto calcV1 = results.first.second;

				auto calcOriginCoord = getCoords(calcOrigin, origin, v1, v2);

				//Make sure that the origin lies on the leg generated by the origin
				Assert::IsTrue(calcOriginCoord.norm() - std::round(calcOriginCoord.norm()) < HEX_ROUNDING_ERROR * v1.norm());

				//Make sure that v1 lies close to the real v1 or its permutations. 
				//We set the origin to 0 so that we just look at the coordinate system as if v1 starts at some 0
				bool trueFlag = false;
				Assert::IsTrue(calcV1.norm() - v1.norm() < v1.norm() * HEX_ROUNDING_ERROR);
				for (int i = 0; i < 6; i++) {
					Eigen::Vector2f potentialV1 = Eigen::Rotation2Df(PI / 3 * i) * v1;
					if ((calcV1 - potentialV1).norm() < HEX_ROUNDING_ERROR* v1.norm())
						trueFlag = true;
				}
				Assert::IsTrue(trueFlag);
				
			}

		}

		TEST_METHOD(HexfitMathematica) {
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
				json resJson({ json(test), json(results.first), json(matrixToVector(results.second.cast<float>())) });
				ret.push_back(resJson);
			}

			std::ofstream f("C:\\Users\\Aiden McIlraith\\Documents\\GitHub\\st-visualizer\\hexResults.json");
			f << ret;

		}
	};
}
