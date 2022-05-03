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
            constexpr int numTests = 20;
            constexpr int testRange = 10;
            constexpr int numPoints = 100;
             //Print out each triangle's points
             auto finalJson = json::array();
             for (int j = 0; j < numTests; j++)
             {
                 auto randPoints = Eigen::Matrix2Xf::Random(2, numPoints) * testRange;
                 const auto out = triangulateMatrix(randPoints);
                 finalJson.push_back(extractTriangleMathematicaMesh(out));
             }

             std::ofstream f(R"(C:\Users\Aiden McIlraith\Documents\GitHub\st-visualizer\triangleResults.json)");
             f << finalJson;
		}
	};
}
