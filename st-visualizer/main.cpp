// This program is from the OpenGL Programming Guide.  It shows a robot arm
// that you can rotate by pressing the arrow keys.

// This program is a flyby around the RGB color cube.  One intersting note
// is that because the cube is a convex polyhedron and it is the only thing
// in the scene, we can render it using backface culling only. i.e., there
// is no need for a depth buffer.


#include <cmath>
#include <fstream>

#include "JSONParser.h"
#include "UtilityFunctions.h"
#include "tetgen1.6.0/tetgen.h"
#include "Contour3D.h"
#include "ImportFunctions.h"
using namespace nlohmann;


//https://wias-berlin.de/software/tetgen/
//https://www.youtube.com/watch?v=A1LqGsyl3C4
//Useful opengl examples: https://cs.lmu.edu/~ray/notes/openglexamples/


//Explore electron as front-end option vs C++ w/ QT

std::vector<std::pair<std::vector<Eigen::Vector3f>, std::vector<std::vector<int>>>>
getVolumeContours(const Eigen::Matrix3Xf& pts, std::vector<std::vector<float>> vals, float shrink)
{
    const auto nmat = vals[0].size();
    tetgenio reg;
    tetralizeMatrix(pts, reg);
    const auto tets = tetgenToTetVector(reg);
    std::vector<Eigen::Vector3f> pts_vector;
    pts_vector.reserve(pts.cols());
    //TODO: Remove the need for the data transform again by using Eigen::Matrix rather than a std::vector of Eigen::Vector
    for (auto& pt : pts.colwise())
    {
        pts_vector.push_back(pt);
    }
    auto [verts, segs, segmats] = contourTetMultiDC(pts_vector, tets, vals);
    return getContourAllMats3D(
        verts, segs, segmats, nmat, shrink);
}


Eigen::Matrix3Xf concatMatrixes(const std::vector<Eigen::Matrix3Xf>& input)
{
    unsigned int sum = 0;
    for (auto& layer : input)
    {
        sum += layer.cols();
    }
    Eigen::Matrix3Xf result(3, sum);
    unsigned int i = 0;
    for (const auto& layer : input)
    {
        for (const auto& pt : layer.colwise())
        {
            result.col(i) = pt;
            i++;
        }
    }
    return result;
}

template <typename T>
std::vector<T> flatten(const std::vector<std::vector<T>>& input)
{
    unsigned int sum = 0;
    for (auto& layer : input)
    {
        sum += layer.size();
    }
    std::vector<T> result;
    result.reserve(sum);
    unsigned int i = 0;
    for (const auto& layer : input)
    {
        for (const auto& pt : layer)
        {
            result.push_back(pt);
        }
    }
    return result;
}


int main(int argc, char* argv[])
{
    std::vector<std::string> a(argv, argv+argc);
    
        constexpr float shrink = 0.04;
        std::vector<std::string> sliceNames({ "NMK_F_U1", "NMK_F_U2", "NMK_F_U3", "NMK_F_U4" });
        std::vector<unsigned> featureCols({ 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 });
        const auto alignmentValues = importAlignments(
            "C:/Users/Aiden McIlraith/Documents/GitHub/st-visualizer/NMK_F_transformation_pt_coord.csv");
        const auto results = loadTsv(
            "C:/Users/Aiden McIlraith/Documents/GitHub/st-visualizer/NMK_20201201_cell_type_coord_allspots.tsv",
            sliceNames,
            1,
            2,
            std::pair<unsigned, unsigned>(3, 4),
            5,
            featureCols,
            60,
            alignmentValues
        );

        auto [ctrs2dVals, tris2dVals] = getSectionContoursAll(results.slices, results.values, shrink);
        auto [ctrs2dclusters, tris2dclusters] = getSectionContoursAll(results.slices, results.clusters, shrink);


        auto allpts = concatMatrixes(results.slices);
        auto ctrs3dVals = getVolumeContours(allpts, flatten<std::vector<float>>(results.values), shrink);
        auto ctrs3dClusters = getVolumeContours(allpts, flatten<std::vector<float>>(results.clusters), shrink);
        auto ptClusIndex = mapVector(results.clusters , std::function([](const std::vector<std::vector<float>>& layer, size_t)
            {
                return mapVector(layer , std::function(getMaxPos));
            }));
        auto ptValIndex = mapVector(results.values, std::function([](const std::vector<std::vector<float>>& layer)
            {
                return mapVector(layer, std::function(getMaxPos));
            }));
        auto slices =mapVector( results.slices , std::function([](const Eigen::Matrix3Xf& layer)
            {
                std::vector<Eigen::Vector3f> temp;
                temp.reserve(layer.cols());
                for (const auto& pt : layer.colwise())
                {
                    temp.emplace_back(pt);
                }
                return temp;
            }));

        auto convertCtrs = [](
            std::vector<std::vector<std::pair<
            std::vector<Eigen::Matrix<float, 3, 1, 0>>, std::vector<std::pair<int, int>>>>>& ctrs2dVals)
        {
            json ctrs2dValsJson = json::array();
            for (auto& ctrSlice : ctrs2dVals)
            {
                json ctrJson = json::array();
                for (auto& ctr : ctrSlice)
                {
                    json temp = json::array();
                    temp.push_back(ctr.first);
                    for (auto& i : ctr.second)
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
            std::vector<int>
            >>&tris2dVals)
        {
            json tris2dValsJson = json::array();
            for (auto& tris : tris2dVals)
            {
                json a = json::array();
                {
                    json b = json::array();
                    for (auto& elem : std::get<0>(tris))
                    {
                        b.push_back(std::vector(elem.data(), elem.data() + elem.rows()));
                    }
                    a.push_back(b);
                }

                auto& triangles = std::get<1>(tris);
                for (auto& tri : triangles)
                {
                    for (auto& ind : tri)
                    {
                        // ind++;
                    }
                }
                a.push_back(triangles);

                auto& materials = std::get<2>(tris);
                for (auto& mat : materials)
                {
// mat++;
                }
                a.push_back(materials);

                tris2dValsJson.push_back(a);
            }

            return tris2dValsJson;
        };

        auto convert3D = [](
            std::vector<std::pair<std::vector<Eigen::Vector3f>, std::vector<std::vector<int>>>>& ctrs3d)
        {
            json ctrs3dJson = json::array();
            for (auto& ctr : ctrs3d)
            {
                json a = json::array();

                a.push_back(ctr.first);
                auto& segs = ctr.second;
                for (auto& a : segs)
                {
                    for (auto& b : a)
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
        ret["nat"]=(results.values[0][0].size()); //nMat,
        ret["shrink"]=(shrink); //shrink,
        ret["clusters"]=(results.clusters); //clusters,
        ret["slices"]=(slices); //slices,
        ret["ptClusIndex"]=(ptClusIndex); //ptClusIndex,
        ret["ctrs2Dvals"]=(convertCtrs(ctrs2dVals)); //ctrs2Dvals,
        ret["ctrs3Dvals"]=(convert3D(ctrs3dVals)); //ctrs3Dvals,
        ret["featureNames"]=(results.names); //featureNames,
        ret["ptValIndex"]=(ptValIndex); //ptValIndex,
        ret["tris2Dvals"]=(convertTris(tris2dVals)); //tris2Dvals
        ret["ctrs2Dclusters"]=(convertCtrs(ctrs2dclusters)); //ctrs2Dclusters,
        ret["ctrs3Dclusters"]=(convert3D(ctrs3dClusters)); //ctrs3Dclusters,
        ret["nClusters"]=(results.clusters[0][0].size()); //nClusters,
        ret["tris2Dclusters"]=(convertTris(tris2dclusters)); //tris2Dclusters,
        ret["featureCols"]=(featureCols); //featureCols,
        ret["sliceNames"]=(sliceNames); //sliceNames
        ret["values"]=(results.values);

        std::ofstream f("C:\\Users\\Aiden McIlraith\\Documents\\GitHub\\st-visualizer\\st-visualizer-electron\\imports\\static\\integrationTest.json");
        f << ret;
        log("Complete. Deconstructing.");
    
}
