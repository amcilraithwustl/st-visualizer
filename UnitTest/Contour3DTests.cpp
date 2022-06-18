#include "pch.h"
#include "CppUnitTest.h"
#include <fstream>
#include "../st-visualizer/UtilityFunctions.h"
#include "../st-visualizer/JSONParser.h"
#include "../st-visualizer/tetgen1.6.0/tetgen.h"
#include "../st-visualizer/Contour3D.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace ContourTests
{
	TEST_CLASS(ContourTests3D)
	{

	public:
		TEST_METHOD(TetgenMathematicaTests)
		{
            srand(0);
            constexpr int numTests = 1;
            constexpr int testRange = 100;
            constexpr int numPoints = 10;
            //Print out each triangle's points
            auto finalJson = json::array();

            {
                auto randPoints = Eigen::Matrix3Xf({
                    {0,1,0,0},
                    {0,0,1,0},
                    {0,0,0,1}
                    });


                tetgenio out;
                tetralizeMatrix(randPoints, out);

                finalJson.push_back(extractTetMathematicaMesh(out));
            }

            for (int j = 0; j < numTests; j++)
            {
                auto randPoints = Eigen::Matrix3Xf::Random(3, numPoints + j * 2) * testRange;
                tetgenio out;
                tetralizeMatrix(randPoints, out);

                finalJson.push_back(extractTetMathematicaMesh(out));
            }

            std::ofstream f(R"(C:\Users\Aiden McIlraith\Documents\GitHub\st-visualizer\tetgenResults.json)");
            f << finalJson;
		}

		TEST_METHOD(ContourMathematicaTests)
		{
			
		}
	};
}
