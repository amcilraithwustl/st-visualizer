#include <string>
// #include "Contour2D.h"
#include <iostream>
#include "JSONParser.h"
#include "UtilityFunctions.h"
using namespace nlohmann;
//https://wias-berlin.de/software/tetgen/


int main(int argc, char** argv)
{
    

    //Print out each triangle's points
    const auto out = triangulateMatrix(Eigen::Matrix2Xf({ {0,0,1,1,-1},{0,1,0,1,-1} }));
    for(int i = 0; i < out.numberoftriangles; i++)
    {
        std::cout << getTriangleMatrix(out, i) << std::endl << std::endl;
    }


    
    // const auto alignmentValues = importAlignments("C:/Users/Aiden McIlraith/Documents/GitHub/st-visualizer/NMK_F_transformation_pt_coord.csv");
    // const auto results = loadTsv(
    // 	"C:/Users/Aiden McIlraith/Documents/GitHub/st-visualizer/NMK_20201201_cell_type_coord_allspots.tsv",
    // 	std::vector<std::string>({ "NMK_F_U1","NMK_F_U2","NMK_F_U3","NMK_F_U4" }),
    // 	1,
    // 	2,
    // 	std::pair<unsigned, unsigned>(3, 
    // 	5,
    // 	std::vector<unsigned>({ 6,7,8,9, 10, 11, 12, 13, 14, 15 }),
    // 	60,
    // 	alignmentValues
    // );

    // getSectionContoursAll(results.slices, results.values, 1);
}
