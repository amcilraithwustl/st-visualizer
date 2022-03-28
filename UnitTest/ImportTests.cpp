#include "pch.h"
#include "../st-visualizer/JSONParser.h"
#include "CppUnitTest.h"
#include "../st-visualizer/GrowAndCover.h"
#include <math.h>;
#include <fstream>;
#include <iostream>
#include <sstream>;
#include "../st-visualizer/ImportFunctions.h"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

using namespace nlohmann;

namespace ImportTests
{
	TEST_CLASS(ImportTests)
	{
	public:
		TEST_METHOD(mathematicaImport)
		{
			const auto alignmentValues = importAlignments("C:/Users/Aiden McIlraith/Documents/GitHub/st-visualizer/CRC112_transformation_pt_coord.csv");
			const auto results = loadTsv(
				"C:/Users/Aiden McIlraith/Documents/GitHub/st-visualizer/CRC_112C1_cell_type_coord_allspots.tsv",
				std::vector<std::string>({ "CRC_HT112C1_1", "CRC_HT112C1_2" }),
				1,
				2,
				std::pair<unsigned, unsigned>(3, 4),
				7,
				std::vector<unsigned>({ 8,9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23 }),
				60,
				alignmentValues
			);

			std::cout << "Completed Successfully" << std::endl;

			json ret = json::array();

			{
				json slices = json::array();
				for (Eigen::Matrix3Xf slice : results.slices)
				{
					json sliceArray = json::array();
					for (Eigen::Vector3f coordinate : slice.colwise())
					{
						json coordArray = json::array();
						coordArray.push_back(coordinate(0));
						coordArray.push_back(coordinate(1));
						coordArray.push_back(coordinate(2));
						sliceArray.push_back(coordArray);
					}
					slices.push_back(sliceArray);
				}
				ret.push_back(slices);
			}

			ret.push_back(results.clusters);
			ret.push_back(results.values);
			ret.push_back(results.names);


			std::ofstream f(R"(C:\Users\Aiden McIlraith\Documents\GitHub\st-visualizer\importTsvResults.json)");
			f << ret;
		}
	};
}