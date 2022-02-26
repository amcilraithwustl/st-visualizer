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
	TEST_CLASS(SVDTests)
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
				Assert::IsTrue(testHexCoord.isApprox(hexCoord), widecstr);
				Assert::IsTrue(testCartPt.isApprox(cartPt), widecstr);
				i++;

			};

			int maxI = 1000;

			//Number of different sets of coordinates to try

			for (int i = 0; i < maxI; i++) {
				//auto v1 = Eigen::Vector2f::Random();
				auto v2 = Eigen::Vector2f::Random();
				auto v1 = Eigen::Vector2f({ 1,0 });
				//auto v2 = Eigen::Vector2f({ 0,1 });
				auto origin = Eigen::Vector2f({0,0});
				//Eigen::Vector2f hexCoord({ (rand() % 1000),(rand() % 1000) });
				Eigen::Vector2f hexCoord({ 1,1 });
				Eigen::Vector2f cartPoint = origin + v1 * 1 + v2 * 1;


				genTest(cartPoint, hexCoord, origin, v1, v2);
			}

		}

	};
}
