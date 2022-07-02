#include <string>
#include <fstream>
#include "JSONParser.h"
#include "UtilityFunctions.h"
#include "ImportFunctions.h"
#include "tetgen1.6.0/tetgen.h"
#include "Contour3D.h"
using namespace nlohmann;
//https://wias-berlin.de/software/tetgen/

//https://www.youtube.com/watch?v=A1LqGsyl3C4

//Useful opengl examples: https://cs.lmu.edu/~ray/notes/openglexamples/
int main(int argc, char* argv[])
{
	using json = nlohmann::json;
	int i = 0;

	auto jsonToMatrix = [](const json& source)
	{
		//This should only be used for testing. Not logic safe.
		std::vector<Eigen::Vector3f> a;
		a.reserve(source.size());
		for (int i = 0; i < source.size(); i++)
		{
			a.emplace_back(
				source[i][0],
				source[i][1],
				source[i][2]
			);
		}
		return a;
	};

	auto jsonToVector = [](const json& source)
	{
		std::vector<std::vector<float>> ret;
		ret.reserve(source.size());
		for (const auto& row : source)
		{
			ret.push_back({});
			std::vector<float>& temp = ret[ret.size()-1];
			temp.reserve(row.size());
			for (const auto& item : row)
			{
				temp.push_back(item);
			}
			ret.push_back(temp);
		}
		return ret;
	};

	auto jsonToTets = [](const json& source)
	{
		std::vector<std::vector<int>> ret;
		ret.reserve(source.size());
		for (auto row : source)
		{
			std::vector<int> temp;
			temp.reserve(row.size());
			for (auto item : row)
			{
				temp.push_back(static_cast<int>(item) - 1);//Subtract 1 b/c mathematica indices start a 1
			}
			ret.push_back(temp);
		}
		return ret;
	};
	// read a JSON file
	std::ifstream file("C:\\Users\\Aiden McIlraith\\Documents\\GitHub\\st-visualizer\\UnitTest\\singleContour3DTest.json");
	json j2 = json::parse(file);
	json ret2 = json::array();
	for (auto& j : j2) {
		std::cout << std::endl << std::endl << "~~ Begin New Evaluation ~~" << std::endl;
		auto pts = jsonToMatrix(j[0]);
        auto vals = jsonToVector(j[1]);
        auto tets = jsonToTets(j[2]);
		j.clear();

		auto randPoints = Eigen::Matrix3Xf(3, pts.size());

		for(int i = 0; i < pts.size(); i++)
		{
			randPoints.col(i) = pts[i];
		}

		tetgenio out;
		tetralizeMatrix(randPoints, out);

        auto [verts, segs, segmats] = contourTetMultiDC(pts, tets, vals);
		auto ctrs = getContourAllMats3D(verts, segs, segmats, vals[0].size(), 0.04);
		json ret = json::array();
		ret.push_back(pts);
		ret.push_back(vals);
		ret.push_back(tets);
		ret.push_back(verts);
		ret.push_back(segs);
		ret.push_back(segmats);
		ret.push_back(ctrs);
		ret.push_back({});
		ret2.push_back(ret);
	}
	std::ofstream f("C:\\Users\\Aiden McIlraith\\Documents\\GitHub\\st-visualizer\\UnitTest\\singleContour3DResults.json");
	f << ret2;

	return 0;
}