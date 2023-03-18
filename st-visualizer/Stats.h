#pragma once
#include "UtilityFunctions.h"
#include <stack>
Eigen::Vector3f cross(const Eigen::Vector3f& A, const Eigen::Vector3f& B, const Eigen::Vector3f& C) {
	return(B - A).cross(C - A);
}

//The positive norm defines the direction that is "positive area"
float triArea(const Eigen::Vector3f& A, const Eigen::Vector3f& B, const Eigen::Vector3f& C, const Eigen::Vector3f& positiveNorm) {
	//One half the cross product is the area of a triangle defined by three points
	auto crossProduct = cross(A, B, C);
	//TODO: Figure out how to make this work Not sure these are all coplanar
	// bool isPositive = crossProduct.normalized() == positiveNorm;
	bool isPositive = true;
	return (0.5f) * crossProduct.norm() * (isPositive ? 1 : -1);
}

std::vector<float> getSurfaceAreas(std::vector<std::pair<std::vector<Eigen::Vector3f>, std::vector<std::vector<int>>>> info) {
	std::vector<float> surfaceAreas = {};
	for (const auto& elem : info) {
		const auto& pts = elem.first;
		const auto& faces = elem.second;
		float surfaceArea = 0;
		for (const auto& face : faces) {
			auto tempSurface = 0;
			Eigen::Vector3f positiveNorm = cross(pts[face[0]], pts[face[1]], pts[face[2]]).normalized();
			for (int i = 1; i < face.size() - 1; i++) {
				tempSurface += triArea(pts[face[0]], pts[face[i]], pts[face[i + 1]], positiveNorm);
			}
			surfaceArea += abs(tempSurface);
		}
		surfaceAreas.push_back(surfaceArea);
	}
	return surfaceAreas;
}


// Volume of a tet w/ vertex at origin is 1/6 the determinant of a 3x3 matrix formed by the remaining vertices
// This volume is signed based on the orientation of the face. This allows us to calculate the total volume of a mesh.
// https://n-e-r-v-o-u-s.com/blog/?p=4415#:~:text=The%20idea%20behind%20calculating%20the,%2C0)%20to%20the%20triangle.

float getTetVolume(const Eigen::Vector3f& A, const Eigen::Vector3f& B, const Eigen::Vector3f& C) {
	return A.cross(B).dot(C) / 6;
}

std::vector<double> getVolumes(std::vector<std::pair<std::vector<Eigen::Vector3f>, std::vector<std::vector<int>>>> info) {
	std::vector<double> volumes = {};
	for (const auto& elem : info) {
		const auto& pts = elem.first;
		const auto& faces = elem.second;
		double volume = 0;
		for (const auto& face : faces) {
			double tempSurface = 0;
			for (int i = 1; i < face.size() - 1; i++) {
				tempSurface += getTetVolume(pts[face[0]], pts[face[i]], pts[face[i + 1]]);
			}
			volume += (tempSurface);
		}
		volumes.push_back(abs(volume));
	}
	return volumes;
}


int countComponents(std::vector<Eigen::Vector3f> points, std::vector<std::vector<int>>faces) {

	std::map<size_t, std::vector<size_t>> facesByPoints;
	for (size_t i = 0; i < faces.size(); i++) {
		const auto& face = faces[i];
		for (const auto& point : face) {
			if (facesByPoints.contains(point)) {
				facesByPoints.at(point).push_back(i);
			}
			else {
				facesByPoints.emplace(point, std::vector({ i }));
			}
		}
	}



	//-1 means unvisited
	std::vector<int> visitedPoints(points.size(), -1);
	std::vector<int> visitedFaces(faces.size(), -1);


	//This tracks which run of the depth first search we are on.
	int currentCluster = 0;

	//This tracks the point each cluster is going to start with. 
	// Always start on 0, then jump to next unclustered point on next run
	size_t startingPoint = 0;


	// We are only going to run this if startingPoint is not 
	// past the end of the vector and it is unvisited to start with
	while (startingPoint < points.size()) {
		//If it is unvisited, find an unvisited point and rerun from the top. This costs O(n) overall.
		if (visitedPoints[startingPoint] != -1) {
			while (startingPoint < points.size() && visitedPoints[startingPoint] != -1) {
				startingPoint++;
			}
			continue;
		}
		//Otherwise

		//This is for each run of the depth first search
		std::stack<size_t> pendingFaces;
		std::stack<size_t> pendingPoints;

		//Set up the initial state by marking and queueing starting point
		visitedPoints[startingPoint] = currentCluster;
		pendingPoints.push(startingPoint);

		// Alternate doing lists until they are both empty
		while (!pendingPoints.empty() || !pendingFaces.empty()) {
			//Do Face
			if (!pendingFaces.empty()) {
				//Get the top
				const auto& top = pendingFaces.top();

				//Mark all connected points and add them to the stack
				for (const auto& point : faces[top]) {
					if (visitedPoints[point] == -1) {
						visitedPoints[point] = currentCluster;
						pendingPoints.push(point);
					}
					else if (visitedPoints[point] != currentCluster) {
						std::cout << "Found error in cluster counting. Results will be innaccurate" << std::endl;
					}
				}
				pendingFaces.pop();
			}

			//Do Point same as above
			if (!pendingPoints.empty()) {
				//Get the top
				const auto& top = pendingPoints.top();

				//Mark all connected faces and add them to the stack
				for (const auto& face : facesByPoints[top]) {
					if (visitedFaces[face] == -1) {
						visitedFaces[face] = currentCluster;
						pendingFaces.push(face);
					}
					else if (visitedFaces[face] != currentCluster) {
						std::cout << "Found error in cluster counting. Results will be innaccurate" << std::endl;
					}
				}
				pendingPoints.pop();
			}
		}
		currentCluster++;
	}
	for (const auto& point : visitedPoints) {
		if (point == -1) 
			throw "Found wrong";
	}
	for (const auto& face : visitedFaces) {
		if (face == -1) 
			throw "Found wrong";
	}
	return currentCluster;
}

std::vector<double> countAllComponents(std::vector<std::pair<std::vector<Eigen::Vector3f>, std::vector<std::vector<int>>>> info) {
	std::vector<double> volumes = {};
	for (const auto& elem : info) {
		const auto& pts = elem.first;
		const auto& faces = elem.second;
		double volume = 0;
		countComponents(pts, faces);
		volumes.push_back(abs(volume));
	}
	return volumes;
}