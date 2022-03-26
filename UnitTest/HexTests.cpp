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
			auto genTest = [&i](Eigen::Vector2f cartPt, Eigen::Vector2f hexCoord, Eigen::Vector2f origin,
			                    Eigen::Vector2f v1, Eigen::Vector2f v2)
			{
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
				auto widestr = std::wstring(failString.begin(), failString.end());
				const wchar_t* widecstr = widestr.c_str();
				Assert::IsTrue(hexCoord.isApprox(testHexCoord, 0.005), widecstr);
				Assert::IsTrue(testCartPt.isApprox(cartPt), widecstr);
				i++;
			};

			int maxI = 5000;

			//Number of different sets of coordinates to try

			for (int i = 0; i < maxI; i++)
			{
				//auto v1 = Eigen::Vector2f::Random();
				Eigen::Vector2f v1 = Eigen::Vector2f::Random();
				Eigen::Vector2f v2 = Eigen::Vector2f::Random();
				Eigen::Vector2f origin = Eigen::Vector2f::Random();
				Eigen::Vector2f hexCoord({(rand() % 1000), (rand() % 1000)});
				Eigen::Vector2f cartPoint = origin + v1 * hexCoord(0) + v2 * hexCoord(1);

				genTest(cartPoint, hexCoord, origin, v1, v2);
			}
		}

		TEST_METHOD(RefineGridMathematica)
		{
			using json = nlohmann::json;
			int i = 0;

			auto jsonToMatrix = [](const json& source)
			{
				//This should only be used for testing. Not logic safe.

				Eigen::Matrix2Xf a(2, source.size());

				for (int i = 0; i < source.size(); i++)
				{
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
			for (json& test : j)
			{
				Eigen::Matrix2Xf pts = jsonToMatrix(test);
				auto gridFound = initGridInliers(pts, 5);
				auto results = refineGrid(pts, gridFound.first, gridFound.second);
				auto getGridResults = getGrid(pts, gridFound.second.first, gridFound.second.second);
				json resJson({
					json(test),
					json({
						json(gridFound.first.first),
						json(gridFound.first.second),
					}),
					json({
						json(getGridResults.first),
						json(getGridResults.second),
					}) ,
					json({
						json(results.first),
						json(results.second),
					})
					});
				ret.push_back(resJson);
			}

			std::ofstream f("C:\\Users\\Aiden McIlraith\\Documents\\GitHub\\st-visualizer\\refineResults.json");
			f << ret;
		}

		TEST_METHOD(HexfitMathematica)
		{
			using json = nlohmann::json;
			int i = 0;

			auto jsonToMatrix = [](const json& source)
			{
				//This should only be used for testing. Not logic safe.

				Eigen::Matrix2Xf a(2, source.size());

				for (int i = 0; i < source.size(); i++)
				{
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
			for (json& test : j)
			{
				Eigen::Matrix2Xf pts = jsonToMatrix(test);
				
				auto results = getGridAndCoords(pts, 5);
				json resJson({json(test), json(results.first), json(matrixToVector(results.second.cast<float>()))});
				ret.push_back(resJson);
			}

			std::ofstream f("C:\\Users\\Aiden McIlraith\\Documents\\GitHub\\st-visualizer\\hexResults.json");
			f << ret;
		}

		TEST_METHOD(GrowAndCoverMathematica)
		{
			using json = nlohmann::json;
			int i = 0;

			auto jsonToMatrix = [](const json& source)
			{
				//This should only be used for testing. Not logic safe.
				Eigen::Matrix2Xf a(2, source.size());

				for (int i = 0; i < source.size(); i++)
				{
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
			for (int k = 0; k < j.size(); k++)
			{
				json test = j[k];
				json sampleJson = k == 0 ? j[k + 1] : j[k - 1];

				Eigen::Matrix2Xf pts = jsonToMatrix(test);
				Eigen::Matrix2Xf sample = jsonToMatrix(sampleJson);
				constexpr unsigned int num = 100;
				std::srand(0);
				auto results0 = growAndCover(pts, sample, 0, num);
				std::srand(0);
				auto results1 = growAndCover(pts, sample, 1, num);
				std::srand(0);
				auto results2 = growAndCover(pts, sample, 2, num);
				json resJson({
					json(test), json(sampleJson), json(matrixToVector(results0)), json(matrixToVector(results1)),
					json(matrixToVector(results2))
				});
				ret.push_back(resJson);
			}

			std::ofstream f(R"(C:\Users\Aiden McIlraith\Documents\GitHub\st-visualizer\growResults.json)");
			f << ret;
		}
	};
}
