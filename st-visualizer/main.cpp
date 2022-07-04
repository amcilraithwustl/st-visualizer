#include "JSONParser.h"
#include "UtilityFunctions.h"
#include "tetgen1.6.0/tetgen.h"
#include "Contour3D.h"
using namespace nlohmann;
//https://wias-berlin.de/software/tetgen/

//https://www.youtube.com/watch?v=A1LqGsyl3C4

//Useful opengl examples: https://cs.lmu.edu/~ray/notes/openglexamples/
int main(int argc, char* argv[])
{
	constexpr auto n = 500000;
    auto rand_pts = Eigen::Matrix3Xf::Random(3, n);
	std::vector<Eigen::Vector3f> pts;
	pts.reserve(n);
	for (int i = 0; i < rand_pts.cols(); i++)
	{
		const auto temp = rand_pts.col(i);
		pts.emplace_back(temp);
	}
	std::vector<std::vector<float>> vals;
	vals.reserve(n);
	for (int i = 0; i < rand_pts.cols(); i++)
	{
        constexpr auto m = 5;
        vals.emplace_back();
		const auto v = Eigen::VectorXf::Random(m);
		for (int i = 0; i < m; i++)
		{
			vals[vals.size() - 1].push_back(v(i));
		}
	}

	tetgenio out;
	tetralizeMatrix(rand_pts, out);
	
	auto tets = tetgenToTetVector(out);
	
	auto [verts, segs, segmats] = contourTetMultiDC(pts, tets, vals);
	auto ctrs = getContourAllMats3D(verts, segs, segmats, vals[0].size(), 0.04);
	

	return 0;
}