#include "pch.h"
#include "CppUnitTest.h"
#include <fstream>
#include "../st-visualizer/UtilityFunctions.h"
#include "../st-visualizer/JSONParser.h"
#include "../st-visualizer/tetgen1.6.0/tetgen.h"
#include "../st-visualizer/Contour3D.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace ContourTests
{
    TEST_CLASS(ContourTests3D)
    {
    private:
        std::vector<std::vector<std::vector<int>>> tets_vec;
        std::vector<std::vector<Eigen::Vector3f>> pts_vec;
        std::vector<std::vector<std::vector<float>>> vals_vec;
        std::vector<std::vector<std::vector<int>>> tetsImported_vec;
    public:
        TEST_METHOD_INITIALIZE(InitValues)
        {
            using json = nlohmann::json;

            if (tets_vec.size()) return;
            tets_vec = {};
            pts_vec = {};
            vals_vec = {};
            tetsImported_vec = {};

            auto jsonToMatrix = [](const json& source)
            {
                //This should only be used for testing. Not logic safe.
                std::vector<Eigen::Vector3f> a;
                a.reserve(source.size());
                for(int i = 0; i < source.size(); i++)
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
                for(auto row : source)
                {
                    std::vector<float> temp;
                    for(auto item : row)
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
                for(auto row : source)
                {
                    std::vector<int> temp;
                    for(auto item : row)
                    {
                        temp.push_back(static_cast<int>(item) - 1); //Subtract 1 b/c mathematica indices start a 1
                    }
                    ret.push_back(temp);
                }
                return ret;
            };
            // read a JSON file
            std::ifstream file(
                "C:\\Users\\Aiden McIlraith\\Documents\\GitHub\\st-visualizer\\UnitTest\\singleContour3DTest.json");
            json j2 = json::parse(file);

            for(const auto& j : j2)
            {
                auto pts = jsonToMatrix(j[0]);
                pts_vec.push_back(pts);
                auto vals = jsonToVector(j[1]);
                vals_vec.push_back(vals);
                auto tetsImported = jsonToTets(j[2]);
                tetsImported_vec.push_back(tetsImported);
                Eigen::Matrix3Xf ptsMat(3, pts.size());
                for(int i = 0; i < pts.size(); i++)
                {
                    ptsMat.col(i) = std::move(pts[i]);
                }
                tetgenio out;
                tetralizeMatrix(ptsMat, out);

                auto tets = jsonToTets(extractTetMathematicaMesh(out)[1]);
                //Undo mathematica index overadjustment from jsonToTets
                for(auto& tet:tets)
                {
                    for(auto& corner:tet)
                    {
                        corner++;
                    }
                }
                tets_vec.push_back(tets);
            }
        }

        TEST_METHOD(Contour3DMathematicaTests)
        {
            using json = nlohmann::json;
            int i = 0;

            json ret2 = json::array();
            for(int i = 0; i < pts_vec.size(); i++)
            {
                auto& pts = pts_vec[i];
                auto& vals = vals_vec[i];
                auto& tets = tets_vec[i];

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
            std::ofstream f(
                "C:\\Users\\Aiden McIlraith\\Documents\\GitHub\\st-visualizer\\UnitTest\\singleContour3DResults.json");
            f << ret2;
        }

        TEST_METHOD(Contour3DMathematicaTestsPregenTets)
        {
            using json = nlohmann::json;
            int i = 0;

            json ret2 = json::array();
            for(int i = 0; i < pts_vec.size(); i++)
            {
                auto& pts = pts_vec[i];
                auto& vals = vals_vec[i];
                auto& tets = tetsImported_vec[i];

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
            std::ofstream f(
                "C:\\Users\\Aiden McIlraith\\Documents\\GitHub\\st-visualizer\\UnitTest\\singleContour3DResults.json");
            f << ret2;
        }


        TEST_METHOD(TetgenMathematicaTests)
        {
            srand(0);
            constexpr int numTests = 1;
            constexpr int testRange = 100;
            constexpr int numPoints = 10;
            //Print out each triangle's points
            auto finalJson = json::array();

            {
                auto randPoints = Eigen::Matrix3Xf({
                    {0, 1, 0, 0},
                    {0, 0, 1, 0},
                    {0, 0, 0, 1}
                });


                tetgenio out;
                tetralizeMatrix(randPoints, out);

                finalJson.push_back(extractTetMathematicaMesh(out));
            }

            for(int j = 0; j < numTests; j++)
            {
                auto randPoints = Eigen::Matrix3Xf::Random(3, numPoints + j * 2) * testRange;
                tetgenio out;
                tetralizeMatrix(randPoints, out);

                finalJson.push_back(extractTetMathematicaMesh(out));
            }

            std::ofstream f(R"(C:\Users\Aiden McIlraith\Documents\GitHub\st-visualizer\tetgenResults.json)");
            f << finalJson;
        }

        TEST_METHOD(ContourMathematicaTests)
        {
        }
    };
}
