#include <vector>
#include "ImportFunctions.h"
#include <string>

int main(int argc, char** argv) {
	std::vector<std::string> arguments(argv, argv + argc);
	auto alignmentValues = importAlignments("../CRC112_transformation_pt_coord.csv");
}