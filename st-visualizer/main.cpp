
#include "Contour3D.h"
#include "ImportFunctions.h"
#include "JSONParser.h"
#include "Stats.h"
#include "UtilityFunctions.h"

#include <fstream>
#include <iostream>
#include <string>

using namespace nlohmann;

using std::string;
using std::vector;
using std::cout;
using std::endl;

// https://wias-berlin.de/software/tetgen/

// argv[1] = 0 use hard coded path, 1 use args[2]
// argv[2] = json
// TODO: Add error handling for json parsing
int main(int argc, char *argv[])
{
    json config;
    if (strcmp(argv[1], "0") == 0)
    {
        config = json::parse(R"(
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
					"zDistance": 100,
                    "resultExport": false,
                    "objExport": false,
                    "featureObj": "bin/features/",
                    "clusterObj": "bin/clusters/"
				}
			)");
//        config = json::parse(R"(
//				{
//					"fileName": "../picture/ToyData/toy_data.tsv",
//					"alignmentFile": "../picture/ToyData/alignment.csv",
//					"target": "../picture/ToyData/output.json",
//					"shrink": 0,
//					"sliceNames": ["slice_1", "slice_2", "slice_3"],
//					"featureCols": [6,7],
//					"sliceIndex": 1,
//					"tissueIndex": 2,
//					"rowIndex": 3,
//					"colIndex": 4,
//					"clusterIndex": 5,
//					"zDistance": 100,
//                    "objExport": false,
//                    "featureObj": "../picture/ToyData/features/",
//                    "clusterObj": "../picture/ToyData/clusters/"
//				}
//			)");
    }
    else if (strcmp(argv[1], "1") == 0)
    {
        config = json::parse(argv[2]);
    }
    else
    {
        std::cout << "Usage: " << argv[0] << " <mode> <json>" << std::endl;
        return 1;
    }

    string alignmentFile = config.at("alignmentFile").get<string>();
    string target = config.at("target").get<string>();
    float shrink = config.at("shrink").get<float>();
    std::cout << "shrink" << shrink << std::endl;
    std::vector<std::string> sliceNames;
    for (const auto &name : config.at("sliceNames"))
    {
        if (name.is_string())
        {
            sliceNames.push_back(name.get<std::string>());
        }
    }
    std::vector<unsigned> featureCols;
    for (const auto &name : config.at("featureCols"))
    {
        featureCols.push_back(name.get<unsigned>());
    }
    std::cout << alignmentFile << std::endl;

    const auto alignmentValues = importAlignments(
        alignmentFile);

    const tsv_return_type results = loadTsv(
        config.at("fileName").get<std::string>(),
        sliceNames,
        config.at("sliceIndex").get<int>(),
        config.at("tissueIndex").get<int>(),
        std::pair<unsigned, unsigned>(config.at("rowIndex").get<int>(), config.at("colIndex").get<int>()),
        config.at("clusterIndex").get<int>(),
        featureCols,
        config.at("zDistance").get<int>(),
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
                                                              for (const auto &pt: layer.colwise())
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

    json ret = json::object();
    ret["nat"] = results.values[0][0].size(); // nMat,
    ret["shrink"] = shrink;                   // shrink,
    ret["clusters"] = results.clusters;       // clusters,
    ret["slices"] = slices;                   // slices,
    ret["sliceNames"] = sliceNames;           // sliceNames
    ret["values"] = results.values;
    ret["featureNames"] = results.names;         // featureNames,
    ret["featureCols"] = featureCols;            // featureCols,
    ret["ptValIndex"] = ptValIndex;              // ptValIndex,
    ret["ctrs2Dvals"] = convertCtrs(ctrs2dVals); // ctrs2Dvals,
    ret["tris2Dvals"] = convertTris(tris2dVals); // tris2Dvals
    ret["ctrs3Dvals"] = convert3D(ctrs3dVals);   // ctrs3Dvals,
//    countAllComponents(ctrs3dVals);
//    ret["ctrsSurfaceAreaVals"] = getSurfaceAreas(ctrs3dVals);
//    ret["ctrsVolumeVals"] = getVolumes(ctrs3dVals);
    ret["ptClusIndex"] = ptClusIndex;                    // ptClusIndex,
    ret["nClusters"] = results.clusters[0][0].size();    // nClusters,
    ret["ctrs2Dclusters"] = convertCtrs(ctrs2dclusters); // ctrs2Dclusters,
    ret["tris2Dclusters"] = convertTris(tris2dclusters); // tris2Dclusters,
    ret["ctrs3Dclusters"] = convert3D(ctrs3dClusters);   // ctrs3Dclusters,
//    ret["ctrsVolumeClusters"] = getVolumes(ctrs3dClusters);

    if (config.at("objExport").get<bool>())
    {
        log("Exporting obj files.");
        exportObj("./bin/features/", ctrs3dVals, results.names);
        exportObj("./bin/clusters/", ctrs3dClusters, results.clusterNames);
    }

    ret["ctrsSurfaceAreaVals"] = computeSurfaceArea(ctrs3dVals);
    ret["ctrsSurfaceAreaClusters"] = computeSurfaceArea(ctrs3dClusters);
    ret["ctrsVolumeVals"] = computeVolume(ctrs3dVals);
    ret["ctrsVolumeClusters"] = computeVolume(ctrs3dClusters);

    cout << "Computing connected components" << endl;
    auto [componentsVals, handlesVals] = connectedComponent(ctrs3dVals);
    auto [componentsClusters, handlesClusters] = connectedComponent(ctrs3dClusters);
    ret["componentsVals"] = componentsVals;
    ret["componentsClusters"] = componentsClusters;
    ret["handlesVals"] = handlesVals;
    ret["handlesClusters"] = handlesClusters;

    cout << ret["componentsVals"] << endl;
    cout << ret["handlesVals"] << endl;
    cout << ret["componentsClusters"] << endl;
    cout << ret["handlesClusters"] << endl;

    log("Calculations complete.");

    if (config.at("resultExport").get<bool>())
    {
        log("Complete. Writing to file.");
        std::ofstream f(target);
        f << ret;
    }

    log("Exiting.");
    return 0;
}
