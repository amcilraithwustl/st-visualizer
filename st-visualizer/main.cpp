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
    auto a = orderTets({ 1,2 }, { 
        {1,2,3,4},
        {0,1,2,3},
        {0,1,2,4},
    });

    return 0;
}