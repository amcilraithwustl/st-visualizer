#include <string>
#include <fstream>
#include "JSONParser.h"
#include "UtilityFunctions.h"
#include "ImportFunctions.h"
#include "tetgen1.6.0/tetgen.h"
#include "Contour3D.h"
using namespace nlohmann;
//https://wias-berlin.de/software/tetgen/



int main(int argc, char* argv[])
{
    srand(0);
    constexpr int numTests = 10;
    constexpr int testRange = 10;
    constexpr int numPoints = 4;
    //Print out each triangle's points
    auto finalJson = json::array();
    for (int j = 0; j < numTests; j++)
    {
        auto randPoints = Eigen::Matrix3Xf::Random(3,numPoints + j) * testRange;
        tetgenio out;
        tetralizeMatrix(randPoints, out);

        finalJson.push_back(extractTetMathematicaMesh(out));
    }

    std::ofstream f(R"(C:\Users\Aiden McIlraith\Documents\GitHub\st-visualizer\tetgenResults.json)");
    f << finalJson;


    return 0;
}