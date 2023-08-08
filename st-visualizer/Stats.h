#pragma once

#include "ImportFunctions.h"
#include "UtilityFunctions.h"

#include <stack>

using std::vector;
using std::pair;

Eigen::Vector3f cross(const Eigen::Vector3f &A, const Eigen::Vector3f &B, const Eigen::Vector3f &C)
{
	return (B - A).cross(C - A);
}

// The positive norm defines the direction that is "positive area"
float triArea(const Eigen::Vector3f &A, const Eigen::Vector3f &B, const Eigen::Vector3f &C, const Eigen::Vector3f &positiveNorm)
{
	// One half the cross product is the area of a triangle defined by three points
	auto crossProduct = cross(A, B, C);
	// TODO: Figure out how to make this work Not sure these are all coplanar
	//  bool isPositive = crossProduct.normalized() == positiveNorm;
	bool isPositive = true;
	return (0.5f) * crossProduct.norm() * (isPositive ? 1 : -1);
}

std::vector<float> getSurfaceAreas(std::vector<std::pair<std::vector<Eigen::Vector3f>, std::vector<std::vector<int>>>> info)
{
	std::vector<float> surfaceAreas = {};
	for (const auto &elem : info)
	{
		const auto &pts = elem.first;
		const auto &faces = elem.second;
		float surfaceArea = 0;
		for (const auto &face : faces)
		{
			auto tempSurface = 0;
			Eigen::Vector3f positiveNorm = cross(pts[face[0]], pts[face[1]], pts[face[2]]).normalized();
			for (int i = 1; i < face.size() - 1; i++)
			{
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

float getTetVolume(const Eigen::Vector3f &A, const Eigen::Vector3f &B, const Eigen::Vector3f &C)
{
	return A.cross(B).dot(C) / 6;
}

std::vector<double> getVolumes(std::vector<std::pair<std::vector<Eigen::Vector3f>, std::vector<std::vector<int>>>> info)
{
	std::vector<double> volumes = {};
	for (const auto &elem : info)
	{
		const auto &pts = elem.first;
		const auto &faces = elem.second;
		double volume = 0;
		for (const auto &face : faces)
		{
			double tempSurface = 0;
			for (int i = 1; i < face.size() - 1; i++)
			{
				tempSurface += getTetVolume(pts[face[0]], pts[face[i]], pts[face[i + 1]]);
			}
			volume += (tempSurface);
		}
		volumes.push_back(abs(volume));
	}
	return volumes;
}

struct countComponentsResult
{
	int componentCount;
	std::vector<std::vector<size_t>> pointsByComponent;
	std::vector<std::vector<size_t>> facesByComponent;
	std::vector<int> componentsByPoint;
	std::vector<int> componentsByFace;
};

countComponentsResult countComponents(std::vector<Eigen::Vector3f> points, std::vector<std::vector<int>> faces)
{

	std::map<size_t, std::vector<size_t>> facesByPoints;
	for (size_t i = 0; i < faces.size(); i++)
	{
		const auto &face = faces[i];
		for (const auto &point : face)
		{
			if (facesByPoints.contains(point))
			{
				facesByPoints.at(point).push_back(i);
			}
			else
			{
				facesByPoints.emplace(point, std::vector({i}));
			}
		}
	}

	//-1 means unvisited
	std::vector<int> componentsByPoint(points.size(), -1);
	std::vector<int> componentsByFace(faces.size(), -1);

	// This tracks which run of the depth first search we are on.
	int currentCluster = 0;

	// This tracks the point each cluster is going to start with.
	//  Always start on 0, then jump to next unclustered point on next run
	size_t startingPoint = 0;

	// We are only going to run this if startingPoint is not
	// past the end of the vector and it is unvisited to start with
	while (startingPoint < points.size())
	{
		// If it is unvisited, find an unvisited point and rerun from the top. This costs O(n) overall.
		if (componentsByPoint[startingPoint] != -1)
		{
			while (startingPoint < points.size() && componentsByPoint[startingPoint] != -1)
			{
				startingPoint++;
			}
			continue;
		}
		// Otherwise

		// This is for each run of the depth first search
		std::stack<size_t> pendingFaces;
		std::stack<size_t> pendingPoints;

		// Set up the initial state by marking and queueing starting point
		componentsByPoint[startingPoint] = currentCluster;
		pendingPoints.push(startingPoint);

		// Alternate doing lists until they are both empty
		while (!pendingPoints.empty() || !pendingFaces.empty())
		{
			// Do Face
			if (!pendingFaces.empty())
			{
				// Get the top
				const auto &top = pendingFaces.top();

				// Mark all connected points and add them to the stack
				for (const auto &point : faces[top])
				{
					if (componentsByPoint[point] == -1)
					{
						componentsByPoint[point] = currentCluster;
						pendingPoints.push(point);
					}
					else if (componentsByPoint[point] != currentCluster)
					{
						std::cout << "Found error in cluster counting. Results will be innaccurate" << std::endl;
					}
				}
				pendingFaces.pop();
			}

			// Do Point same as above
			if (!pendingPoints.empty())
			{
				// Get the top
				const auto &top = pendingPoints.top();

				// Mark all connected faces and add them to the stack
				for (const auto &face : facesByPoints[top])
				{
					if (componentsByFace[face] == -1)
					{
						componentsByFace[face] = currentCluster;
						pendingFaces.push(face);
					}
					else if (componentsByFace[face] != currentCluster)
					{
						std::cout << "Found error in cluster counting. Results will be innaccurate" << std::endl;
					}
				}
				pendingPoints.pop();
			}
		}
		currentCluster++;
	}
	for (const auto &point : componentsByPoint)
	{
		if (point == -1)
			throw "Found wrong";
	}
	for (const auto &face : componentsByFace)
	{
		if (face == -1)
			throw "Found wrong";
	}
	countComponentsResult res;
	res.componentCount = currentCluster;
	res.facesByComponent = std::vector(currentCluster, std::vector<size_t>({}));
	for (size_t face = 0; face < faces.size(); face++)
	{
		res.facesByComponent[componentsByFace[face]].push_back(face);
	}

	res.pointsByComponent = std::vector(currentCluster, std::vector<size_t>({}));
	for (size_t point = 0; point < points.size(); point++)
	{
		res.pointsByComponent[componentsByPoint[point]].push_back(point);
	}
	res.componentsByPoint = componentsByPoint;
	res.componentsByFace = componentsByFace;
	return res;
}
struct numVEF
{
	unsigned int numEdges;
	unsigned int numFaces;
	unsigned int numVertices;
};
std::vector<numVEF> countEdgesFacesVerticesPerComponent(countComponentsResult components, std::vector<std::vector<int>> faces)
{
	std::vector<numVEF> ret;
	for (int c = 0; c < components.componentCount; c++)
	{

		// Vertices in component. Trivial
		size_t numVertices = components.pointsByComponent[c].size();

		// Faces in component. Trivial
		size_t numFaces = components.facesByComponent[c].size();

		// Edges. Count around each face in component, divide by two;
		int numEdges = 0;
		for (const auto &face : components.facesByComponent[c])
		{
			numEdges += faces[face].size();
		}

		numVEF temp = {};
		temp.numVertices = numVertices;
		temp.numFaces = numFaces;
		temp.numEdges = numEdges / 2;
		ret.push_back(temp);
	}

	return ret;
}

// Returns the euler characteristic for each component for each featuremesh as nested vectors of ints
vector<vector<int>> countAllComponents(vector<std::pair<std::vector<Eigen::Vector3f>, std::vector<std::vector<int>>>> featureMesh)
{
	vector<std::vector<int>> volumes = {};
	for (const auto &[pts, faces] : featureMesh)
	{
        countComponentsResult components = countComponents(pts, faces);
		vector<numVEF> vefList = countEdgesFacesVerticesPerComponent(components, faces);
		vector<int> eulerCharacteristics;
		for (const auto &vef : vefList)
		{
			eulerCharacteristics.push_back(vef.numVertices - vef.numEdges + vef.numFaces);
		}
		volumes.push_back(eulerCharacteristics);
	}
	return volumes;
}

vector<pair<vector<Eigen::Vector3f>, vector<vector<int>>>>
getVolumeContours(const Eigen::Matrix3Xf &pts, vector<vector<float>> vals, float shrink)
{
    const size_t nmat = vals[0].size();
    tetgenio reg;
    tetralizeMatrix(pts, reg);
    const vector<vector<int>> tets = tetgenToTetVector(reg);
    vector<Eigen::Vector3f> pts_vector;
    pts_vector.reserve(pts.cols());
    // TODO: Remove the need for the data transform again by using Eigen::Matrix rather than a std::vector of Eigen::Vector
    for (auto &pt : pts.colwise())
    {
        pts_vector.emplace_back(pt);
    }
    auto [verts, segs, segmats] = contourTetMultiDC(pts_vector, tets, vals);
    return getContourAllMats3D(verts, segs, segmats, nmat, shrink);
}

Eigen::Matrix3Xf concatMatrixes(const vector<Eigen::Matrix3Xf> &input)
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