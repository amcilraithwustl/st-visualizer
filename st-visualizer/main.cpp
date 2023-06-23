#include "Contour3D.h"
#include "ImportFunctions.h"
#include "JSONParser.h"
#include "Stats.h"
#include "UtilityFunctions.h"

#include <cmath>
#include <fstream>
#include <iostream>
#include <string>

using namespace nlohmann;

using std::string;
using std::vector;

// https://wias-berlin.de/software/tetgen/
// https://www.youtube.com/watch?v=A1LqGsyl3C4
// Useful opengl examples: https://cs.lmu.edu/~ray/notes/openglexamples/

// Explore electron as front-end option vs C++ w/ QT

std::vector<std::pair<std::vector<Eigen::Vector3f>, std::vector<std::vector<int>>>>
getVolumeContours(const Eigen::Matrix3Xf &pts, std::vector<std::vector<float>> vals, float shrink)
{
	const auto nmat = vals[0].size();
	tetgenio reg;
	tetralizeMatrix(pts, reg);
	const auto tets = tetgenToTetVector(reg);
	std::vector<Eigen::Vector3f> pts_vector;
	pts_vector.reserve(pts.cols());
	// TODO: Remove the need for the data transform again by using Eigen::Matrix rather than a std::vector of Eigen::Vector
	for (auto &pt : pts.colwise())
	{
		pts_vector.push_back(pt);
	}
	auto [verts, segs, segmats] = contourTetMultiDC(pts_vector, tets, vals);
	return getContourAllMats3D(
		verts, segs, segmats, nmat, shrink);
}

Eigen::Matrix3Xf concatMatrixes(const std::vector<Eigen::Matrix3Xf> &input)
{
	unsigned int sum = 0;
	for (auto &layer : input)
	{
		sum += layer.cols();
	}
	Eigen::Matrix3Xf result(3, sum);
	unsigned int i = 0;
	for (const auto &layer : input)
	{
		for (const auto &pt : layer.colwise())
		{
			result.col(i) = pt;
			i++;
		}
	}
	return result;
}

template <typename T>
std::vector<T> flatten(const std::vector<std::vector<T>> &input)
{
	unsigned int sum = 0;
	for (auto &layer : input)
	{
		sum += layer.size();
	}
	std::vector<T> result;
	result.reserve(sum);
	unsigned int i = 0;
	for (const auto &layer : input)
	{
		for (const auto &pt : layer)
		{
			result.push_back(pt);
		}
	}
	return result;
}

// argv[1] = 0 use hard coded path, 1 use args[2]
// argv[2] = json
// TODO: Add error handling for json parsing
int main(int argc, char *argv[])
{

	json tempFile;
	if (strcmp(argv[1], "0") == 0)
	{
		tempFile = json::parse(R"(
				{
					"fileName": "../picture/AlignmentImages/NMKimage/NMK_20201201_cell_type_coord_allspots.tsv",
					"alignmentFile": "../data/NMK_F_transformation_pt_coord.csv",
					"target": "bin/nmk-test-output.json",
					"shrink": 0,
					"sliceNames": ["NMK_F_U1","NMK_F_U2","NMK_F_U3","NMK_F_U4"],
					"featureCols": [6,7,8,9,10,11,12,13,14,15],
					"sliceIndex": 1,
					"tissueIndex": 2,
					"rowIndex": 3,
					"colIndex": 4,
					"clusterIndex": 5,
					"zDistance": 100
				}
			)");
	}
	else if (strcmp(argv[1], "1") == 0)
	{
		tempFile = json::parse(argv[2]);
	}
	else
	{
		std::cout << "Usage: " << argv[0] << " <mode> <json>" << std::endl;
		return 1;
	}

	string alignmentFile = tempFile.at("alignmentFile").get<string>();
	string target = tempFile.at("target").get<string>();

	float shrink = tempFile.at("shrink").get<float>();
	std::cout << "shrink" << shrink << std::endl;
	std::vector<std::string> sliceNames;
	for (const auto &name : tempFile.at("sliceNames"))
	{
		if (name.is_string())
		{
			sliceNames.push_back(name.get<std::string>());
		}
	} //({"NMK_F_U1", "NMK_F_U2", "NMK_F_U3", "NMK_F_U4"});
	std::vector<unsigned> featureCols;
	for (const auto &name : tempFile.at("featureCols"))
	{
		featureCols.push_back(name.get<unsigned>());
	}
	std::cout << alignmentFile << std::endl;

	const auto alignmentValues = importAlignments(
		alignmentFile);

	const auto results = loadTsv(
		tempFile.at("fileName").get<std::string>(),
		sliceNames,
		tempFile.at("sliceIndex").get<int>(),
		tempFile.at("tissueIndex").get<int>(),
		std::pair<unsigned, unsigned>(tempFile.at("rowIndex").get<int>(), tempFile.at("colIndex").get<int>()),
		tempFile.at("clusterIndex").get<int>(),
		featureCols,
		tempFile.at("zDistance").get<int>(),
		alignmentValues);

	auto [ctrs2dVals, tris2dVals] = getSectionContoursAll(results.slices, results.values, shrink);
	auto [ctrs2dclusters, tris2dclusters] = getSectionContoursAll(results.slices, results.clusters, shrink);

	auto allpts = concatMatrixes(results.slices);
	auto ctrs3dVals = getVolumeContours(allpts, flatten<std::vector<float>>(results.values), shrink);
	auto ctrs3dClusters = getVolumeContours(allpts, flatten<std::vector<float>>(results.clusters), shrink);
	auto ptClusIndex = mapVector(results.clusters, std::function(
													   [](const std::vector<std::vector<float>> &layer, size_t)
													   {
														   return mapVector(layer, std::function(getMaxPos));
													   }));
	auto ptValIndex = mapVector(results.values, std::function([](const std::vector<std::vector<float>> &layer)
															  { return mapVector(layer, std::function(getMaxPos)); }));
	auto slices = mapVector(results.slices, std::function([](const Eigen::Matrix3Xf &layer)
														  {
			std::vector<Eigen::Vector3f> temp;
			temp.reserve(layer.cols());
			for (const auto& pt : layer.colwise())
			{
				temp.emplace_back(pt);
			}
			return temp; }));

	auto convertCtrs = [](
						   std::vector<std::vector<std::pair<
							   std::vector<Eigen::Matrix<float, 3, 1, 0>>, std::vector<std::pair<int, int>>>>> &ctrs2dVals)
	{
		json ctrs2dValsJson = json::array();
		for (auto &ctrSlice : ctrs2dVals)
		{
			json ctrJson = json::array();
			for (auto &ctr : ctrSlice)
			{
				json temp = json::array();
				temp.push_back(ctr.first);
				for (auto &i : ctr.second)
				{
					// i.first++;
					// i.second++;
				}
				temp.push_back(ctr.second);
				ctrJson.push_back(temp);
			}

			ctrs2dValsJson.push_back(ctrJson);
		}

		return ctrs2dValsJson;
	};

	auto convertTris = [](std::vector<std::tuple<
							  std::vector<Eigen::Matrix<float, 3, 1, 0>>,
							  std::vector<std::vector<int>>,
							  std::vector<int>>> &tris2dVals)
	{
		json tris2dValsJson = json::array();
		for (auto &tris : tris2dVals)
		{
			json a = json::array();
			{
				json b = json::array();
				for (auto &elem : std::get<0>(tris))
				{
					b.push_back(std::vector(elem.data(), elem.data() + elem.rows()));
				}
				a.push_back(b);
			}

			auto &triangles = std::get<1>(tris);
			for (auto &tri : triangles)
			{
				for (auto &ind : tri)
				{
					// ind++;
				}
			}
			a.push_back(triangles);

			auto &materials = std::get<2>(tris);
			for (auto &mat : materials)
			{
				// mat++;
			}
			a.push_back(materials);

			tris2dValsJson.push_back(a);
		}

		return tris2dValsJson;
	};

	auto convert3D = [](
						 std::vector<std::pair<std::vector<Eigen::Vector3f>, std::vector<std::vector<int>>>> &ctrs3d)
	{
		json ctrs3dJson = json::array();
		for (auto &ctr : ctrs3d)
		{
			json a = json::array();

			a.push_back(ctr.first);
			auto &segs = ctr.second;
			for (auto &a : segs)
			{
				for (auto &b : a)
				{
					// b++;
				}
			}
			a.push_back(segs);

			ctrs3dJson.push_back(a);
		}

		return ctrs3dJson;
	};
	log("Calculations complete. Writing to file.");
	json ret = json::object();
	ret["nat"] = results.values[0][0].size(); // nMat,
	ret["shrink"] = shrink;					  // shrink,
	ret["clusters"] = results.clusters;		  // clusters,
	ret["slices"] = slices;					  // slices,
	ret["sliceNames"] = sliceNames;			  // sliceNames
	ret["values"] = results.values;
	ret["featureNames"] = results.names;		 // featureNames,
	ret["featureCols"] = featureCols;			 // featureCols,
	ret["ptValIndex"] = ptValIndex;				 // ptValIndex,
	ret["ctrs2Dvals"] = convertCtrs(ctrs2dVals); // ctrs2Dvals,
	ret["tris2Dvals"] = convertTris(tris2dVals); // tris2Dvals
	ret["ctrs3Dvals"] = convert3D(ctrs3dVals);	 // ctrs3Dvals,
	countAllComponents(ctrs3dVals);
	ret["ctrsSurfaceAreaVals"] = getSurfaceAreas(ctrs3dVals);
	ret["ctrsVolumeVals"] = getVolumes(ctrs3dVals);
	ret["ptClusIndex"] = ptClusIndex;					 // ptClusIndex,
	ret["nClusters"] = results.clusters[0][0].size();	 // nClusters,
	ret["ctrs2Dclusters"] = convertCtrs(ctrs2dclusters); // ctrs2Dclusters,
	ret["tris2Dclusters"] = convertTris(tris2dclusters); // tris2Dclusters,
	ret["ctrs3Dclusters"] = convert3D(ctrs3dClusters);	 // ctrs3Dclusters,
	ret["ctrsVolumeClusters"] = getVolumes(ctrs3dClusters);
	// ret["volumeContours"] = getVolumeContours()

	std::ofstream f(target);
	f << ret;
	log("Complete. Deconstructing.");
}
