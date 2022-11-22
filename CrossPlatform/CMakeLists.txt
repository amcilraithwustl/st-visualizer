# CMakeList.txt : CMake project for CrossPlatform, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("st-visualizer")

add_compile_options(-std=c++2a)

#find_package(Eigen REQUIRED)

#link_directories("./Eigen/")

# Add source to this project's executable.
add_executable (stvisualizer "main.cpp" "main.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET stvisualizer PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
#target_link_libraries(${PROJECT_NAME} ${Eigen_LIBRARIES})