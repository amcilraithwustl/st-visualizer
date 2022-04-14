#include <string>
// #include "Contour2D.h"
#include "JSONParser.h"
#include "triangle-1.6/triangle.h"
using namespace nlohmann;
//https://wias-berlin.de/software/tetgen/


int main(int argc, char** argv)
{
    //TODO: Investigate flags further
    std::string flags =
            std::string("z") //Start arrays at 0
            + std::string("V") //Verbose
            + std::string("Q") //Quiet
        ;

    double points[6] = {1, 2, -3, 4, 5, -6};
    triangulateio in = {};
    in.pointlist = points;
    in.numberofpoints = 3;
    in.numberofpointattributes = 0;
    in.pointmarkerlist = NULL; //Might be able to use this to associate points with indices in the original matrix
    in.pointattributelist = NULL; //Might be able to use this to associate points with indices in the original matrix

    triangulateio out = {};
    out.pointlist = NULL;
    out.trianglelist = NULL;
    out.pointmarkerlist = NULL;
    out.pointattributelist = NULL;
    out.trianglelist = NULL;

    // triangulate(&flags[0], &in, &out, nullptr);
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
