#include <string>
// #include "Contour2D.h"
#include <fstream>
#include <iostream>
#include "JSONParser.h"
#include "UtilityFunctions.h"
using namespace nlohmann;
//https://wias-berlin.de/software/tetgen/

using json = nlohmann::json;
template<typename T, int G>
std::vector<T> eigenToVec(Eigen::Vector<T, G> mat) {return std::vector<T>( mat.data(), mat.data() + mat.rows()); }

template<typename T>
json toJson(std::vector<T> v)
{
    auto a = json::array();
    for(size_t i = 0; i < v.size(); i++)
    {
        a.push_back(v[i]);
    }
    return a;
}

json extractTriangleMathematicaMesh(const triangulateio& obj)
{
    const auto points = table(obj.numberofpoints, std::function([obj](size_t i)
    {
        return getCornerVector(obj, i);
    }));
    const auto triangles = table(obj.numberoftriangles, std::function([obj](size_t i) {return getTriangleCornerIndices(obj, i); }));

    
    json pointJson = json::array();
    for (auto& pt : points)
    {
        std::cout << pt << std::endl;
        pointJson.push_back(toJson(eigenToVec(pt)));
    }

    json triangleJson = json::array();
    for (auto& set : triangles)
    {
        triangleJson.push_back(toJson(set));
    }
    json ret = json::array({ pointJson ,triangleJson });
    return ret;
};

int main(int argc, char** argv)
{
    

    //Print out each triangle's points
    const auto out = triangulateMatrix(Eigen::Matrix2Xf({ {0,0,1,1,-1},{0,1,0,1,-1} }));
    for(int i = 0; i < out.numberoftriangles; i++)
    {
        std::cout << getTriangleMatrix(out, i) << std::endl << std::endl;
    }

   
    const auto a = extractTriangleMathematicaMesh(out);

    std::cout << a << std::endl;

    std::ofstream f(R"(C:\Users\Aiden McIlraith\Documents\GitHub\st-visualizer\triangleResults.json)");
    f << a;
    

	

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
