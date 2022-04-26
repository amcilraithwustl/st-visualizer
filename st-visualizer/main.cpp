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
    auto finalJson = json::array();
    for(int j = 0; j < 10; j++)
    {
        auto randPoints = Eigen::Matrix2Xf::Random(2, 100) * 10;
        const auto out = triangulateMatrix(randPoints);
        finalJson.push_back(extractTriangleMathematicaMesh(out));
    }
    std::cout << finalJson << std::endl;

    std::ofstream f(R"(C:\Users\Aiden McIlraith\Documents\GitHub\st-visualizer\triangleResults.json)");
    f << finalJson;
    

	

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
