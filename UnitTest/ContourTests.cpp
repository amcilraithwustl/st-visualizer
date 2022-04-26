#include "pch.h"
#include "CppUnitTest.h"
#include <fstream>
#include "../st-visualizer/UtilityFunctions.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace ContourTests
{
	TEST_CLASS(ContourTests)
	{
	public:
		TEST_METHOD(DelauneyMathematicaTests)
		{
             //Print out each triangle's points
             auto finalJson = json::array();
             for (int j = 0; j < 20; j++)
             {
                 auto randPoints = Eigen::Matrix2Xf::Random(2, 100) * 10;
                 const auto out = triangulateMatrix(randPoints);
                 finalJson.push_back(extractTriangleMathematicaMesh(out));
             }

             std::ofstream f(R"(C:\Users\Aiden McIlraith\Documents\GitHub\st-visualizer\triangleResults.json)");
             f << finalJson;
		}
	};
}
