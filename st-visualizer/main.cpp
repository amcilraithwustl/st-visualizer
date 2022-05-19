#include <string>
#include <fstream>
#include <iostream>
#include "Contour2D.h"
#include "JSONParser.h"
#include "UtilityFunctions.h"
#include "ImportFunctions.h"
using namespace nlohmann;
//https://wias-berlin.de/software/tetgen/


int main(int argc, char** argv)
{
	using json = json;

	const auto alignmentValues = importAlignments("C:/Users/Aiden McIlraith/Documents/GitHub/st-visualizer/NMK_F_transformation_pt_coord.csv");
	
    const auto results = loadTsv(
	 	"C:/Users/Aiden McIlraith/Documents/GitHub/st-visualizer/NMK_20201201_cell_type_coord_allspots.tsv",
	 	std::vector<std::string>({ "NMK_F_U1","NMK_F_U2","NMK_F_U3","NMK_F_U4" }),
	 	1,
	 	2,
	 	std::pair<unsigned, unsigned>(3, 4),
	 	5,
	 	std::vector<unsigned>({ 6,7,8,9, 10, 11, 12, 13, 14, 15 }),
	 	60,
	 	alignmentValues
	 );
	



	// read a JSON file
	
	json ret2 = json::array();

	//First, clusters
	for (size_t i = 0; i < results.slices.size(); i++)
	{
		auto pts = results.slices[i].topRows(2);
		auto vals = results.clusters[i];
		auto delaunay = triangulateMatrix(pts);
		
		auto tris = table(static_cast<size_t>(delaunay.numberoftriangles), std::function([delaunay](size_t i)
			{
				return getTriangleCornerIndices(delaunay, i);
			}));

		auto first = contourTriMultiDC(pts, tris, vals);

		json ret = json::array();
		ret.push_back(first.verts);
		ret.push_back(first.segs);
		ret.push_back(first.fillVerts);
		ret.push_back(first.fillTris);
		ret.push_back(first.fillMats);
		ret2.push_back(ret);
	}
	
	//Second, values
	for (size_t i = 0; i < results.slices.size(); i++)
	{
		auto pts = results.slices[i].topRows(2);
		auto vals = results.values[i];
		auto delaunay = triangulateMatrix(pts);
		auto tris = table(static_cast<size_t>(delaunay.numberoftriangles), std::function([delaunay](size_t i)
			{
				return getTriangleCornerIndices(delaunay, i);
			}));

		auto first = contourTriMultiDC(pts, tris, vals);

		json ret = json::array();
		ret.push_back(first.verts);
		ret.push_back(first.segs);
		ret.push_back(first.fillVerts);
		ret.push_back(first.fillTris);
		ret.push_back(first.fillMats);
		ret2.push_back(ret);
	}
	


	std::ofstream f(
		"C:\\Users\\Aiden McIlraith\\Documents\\GitHub\\st-visualizer\\UnitTest\\realContourResultsFaces.json");
	f << ret2;
	
}
