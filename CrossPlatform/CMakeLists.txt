# CMakeList.txt : CMake project for CrossPlatform, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.12)

project ("st-visualizer")

find_package(Eigen3 3.4 REQUIRED NO_MODULE)

add_subdirectory(tetgen1.6.0)

# Add source to this project's executable.
add_executable (stvisualizer "main.cpp" "main.h" 
	"Contour2D.cpp" "Contour2D.cpp" "GrowAndCover.cpp" "ImportFunctions.cpp" "UtilityFunctions.cpp"
	"Contour2D.h"   "Contour2D.h"   "GrowAndCover.h"   "ImportFunctions.h"   "UtilityFunctions.h"
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET stvisualizer PROPERTY CXX_STANDARD 20)
endif()

target_link_libraries(stvisualizer Eigen3::Eigen)

# TODO: Add tests and install targets if needed.
#target_link_libraries(${PROJECT_NAME} ${Eigen_LIBRARIES})