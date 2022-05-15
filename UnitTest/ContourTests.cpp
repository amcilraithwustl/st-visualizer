#include "pch.h"
#include "CppUnitTest.h"
#include <fstream>
#include "../st-visualizer/UtilityFunctions.h"
#include <fstream>
#include "../st-visualizer/Contour2D.h"
#include "../st-visualizer/JSONParser.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace ContourTests
{
	TEST_CLASS(ContourTests)
	{
	public:
		TEST_METHOD(DelauneyMathematicaTests)
		{
            constexpr int numTests = 20;
            constexpr int testRange = 10;
            constexpr int numPoints = 100;
             //Print out each triangle's points
             auto finalJson = json::array();
             for (int j = 0; j < numTests; j++)
             {
                 auto randPoints = Eigen::Matrix2Xf::Random(2, numPoints) * testRange;
                 const auto out = triangulateMatrix(randPoints);
                 finalJson.push_back(extractTriangleMathematicaMesh(out));
             }

             std::ofstream f(R"(C:\Users\Aiden McIlraith\Documents\GitHub\st-visualizer\triangleResults.json)");
             f << finalJson;
		}
        TEST_METHOD(TriangulateAndContourMathematicaTests)
		{
			using json = nlohmann::json;
			int i = 0;

			auto jsonToMatrix = [](const json& source)
			{
				//This should only be used for testing. Not logic safe.
				Eigen::Matrix2Xf a(2, source.size());

				for (int i = 0; i < source.size(); i++)
				{
					a(0, i) = source[i][0];
					a(1, i) = source[i][1];
				}
				return a;
			};

			auto jsonToVector = [](const json& source)
			{
				std::vector<std::vector<float>> ret;
				for (auto row : source)
				{
					std::vector<float> temp;
					for (auto item : row)
					{
						temp.push_back(item);
					}
					ret.push_back(temp);
				}
				return ret;
			};

			auto jsonToTris = [](const json& source)
			{
				std::vector<std::vector<int>> ret;
				for (auto row : source)
				{
					std::vector<int> temp;
					for (auto item : row)
					{
						temp.push_back(static_cast<int>(item) - 1);//Subtract 1 b/c mathematica indices start a 1
					}
					ret.push_back(temp);
				}
				return ret;
			};
			// read a JSON file
			std::ifstream file("C:\\Users\\Aiden McIlraith\\Documents\\GitHub\\st-visualizer\\UnitTest\\singleContourTest.json");
			auto a = file.is_open();
			json j2 = json::parse(file);
			json ret2 = json::array();
			for (auto j : j2) {
				auto pts = jsonToMatrix(j[0]);
				auto vals = jsonToVector(j[1]);
				auto delaunay = triangulateMatrix(pts);
				auto tris = table(static_cast<size_t>(delaunay.numberoftriangles),std::function([delaunay](size_t i) {return getTriangleCornerIndices(delaunay, i); }));

				auto first = contourTriMultiDC(pts, tris, vals);

				json ret = json::array();

				std::vector<Eigen::Vector2f> temp;
				for (auto& vert : first.verts)
				{
					temp.push_back(vert);
				}
				ret.push_back(temp);
				ret.push_back(first.segs);
				ret2.push_back(ret);
			}
			std::ofstream f("C:\\Users\\Aiden McIlraith\\Documents\\GitHub\\st-visualizer\\UnitTest\\singleContourResults.json");
			f << ret2;
		}
	    TEST_METHOD(ContourMathematicaTests)
		{
			using json = nlohmann::json;
			int i = 0;

			auto jsonToMatrix = [](const json& source)
			{
				//This should only be used for testing. Not logic safe.
				Eigen::Matrix2Xf a(2, source.size());

				for (int i = 0; i < source.size(); i++)
				{
					a(0, i) = source[i][0];
					a(1, i) = source[i][1];
				}
				return a;
			};

			auto jsonToVector = [](const json& source)
			{
				std::vector<std::vector<float>> ret;
				for (auto row : source)
				{
					std::vector<float> temp;
					for (auto item : row)
					{
						temp.push_back(item);
					}
					ret.push_back(temp);
				}
				return ret;
			};

			auto jsonToTris = [](const json& source)
			{
				std::vector<std::vector<int>> ret;
				for (auto row : source)
				{
					std::vector<int> temp;
					for (auto item : row)
					{
						temp.push_back(static_cast<int>(item) - 1);//Subtract 1 b/c mathematica indices start a 1
					}
					ret.push_back(temp);
				}
				return ret;
			};
			// read a JSON file
			std::ifstream file("C:\\Users\\Aiden McIlraith\\Documents\\GitHub\\st-visualizer\\UnitTest\\singleContourTest.json");
			auto a = file.is_open();
			json j2 = json::parse(file);
			json ret2 = json::array();
			for (auto j : j2) {
				auto pts = jsonToMatrix(j[0]);
				auto vals = jsonToVector(j[1]);
				auto tris = jsonToTris(j[2]);

				auto first = contourTriMultiDC(pts, tris, vals);

				json ret = json::array();

				std::vector<Eigen::Vector2f> temp;
				for (auto& vert : first.verts)
				{
					temp.push_back(vert);
				}
				ret.push_back(temp);
				ret.push_back(first.segs);
				ret2.push_back(ret);
			}
			std::ofstream f("C:\\Users\\Aiden McIlraith\\Documents\\GitHub\\st-visualizer\\UnitTest\\singleContourResults.json");
			f << ret2;
		}

		TEST_METHOD(TriangulateAndContourMathematicaFaceTests)
		{
			using json = nlohmann::json;
			int i = 0;

			auto jsonToMatrix = [](const json& source)
			{
				//This should only be used for testing. Not logic safe.
				Eigen::Matrix2Xf a(2, source.size());

				for (int i = 0; i < source.size(); i++)
				{
					a(0, i) = source[i][0];
					a(1, i) = source[i][1];
				}
				return a;
			};

			auto jsonToVector = [](const json& source)
			{
				std::vector<std::vector<float>> ret;
				for (auto row : source)
				{
					std::vector<float> temp;
					for (auto item : row)
					{
						temp.push_back(item);
					}
					ret.push_back(temp);
				}
				return ret;
			};

			auto jsonToTris = [](const json& source)
			{
				std::vector<std::vector<int>> ret;
				for (auto row : source)
				{
					std::vector<int> temp;
					for (auto item : row)
					{
						temp.push_back(static_cast<int>(item) - 1);//Subtract 1 b/c mathematica indices start a 1
					}
					ret.push_back(temp);
				}
				return ret;
			};
			// read a JSON file
			std::ifstream file("C:\\Users\\Aiden McIlraith\\Documents\\GitHub\\st-visualizer\\UnitTest\\singleContourTest.json");
			auto a = file.is_open();
			json j2 = json::parse(file);
			json ret2 = json::array();
			for (auto j : j2) {
				auto pts = jsonToMatrix(j[0]);
				auto vals = jsonToVector(j[1]);
				auto delaunay = triangulateMatrix(pts);
				auto tris = table(static_cast<size_t>(delaunay.numberoftriangles), std::function([delaunay](size_t i) {return getTriangleCornerIndices(delaunay, i); }));

				auto first = contourTriMultiDC(pts, tris, vals);

				json ret = json::array();

				std::vector<Eigen::Vector2f> temp;
				for (auto& vert : first.fillVerts)
				{
					temp.push_back(vert);
				}
				ret.push_back(temp);

				//Adjust indices for mathematica indexing
				for(auto& tri : first.fillTris)
				{
					for(auto& index:tri)
					{
						index++;
					}
				}
				ret.push_back(first.fillTris);

				ret.push_back(first.fillMats);
				ret2.push_back(ret);
			}
			std::ofstream f("C:\\Users\\Aiden McIlraith\\Documents\\GitHub\\st-visualizer\\UnitTest\\singleContourResultsFaces.json");
			f << ret2;
		}

	};
}
