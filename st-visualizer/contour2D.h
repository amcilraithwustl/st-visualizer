/*
#pragma once
#include <vector>
#include "triangle-1.6/triangle.h"
#include "UtilityFunctions.h"
// orientation (not sure if this should be i or f)
// Returns 1 if left, 0 if none, -1 if right (right hand rule)
int orientation(Eigen::Vector2f a, Eigen::Vector2f b);

// getMaxPos
int getMaxPos(const std::vector<float>& vals);

// getMassPoint
// Might need work based on input type
Eigen::Vector2f inline getMassPoint(Eigen::Matrix2Xf m){return m.rowwise().mean();}

// InterpEdge2Mat
//Might be f or i inputs, but must be f output
Eigen::Vector2f interpEdge2Mat(const Eigen::Vector2f& p, const Eigen::Vector2f& q, std::pair<float, float> p_vals, std::pair<float, float> q_vals)
{
    const float t = std::abs(p_vals.first - p_vals.second) / (std::abs(p_vals.first - p_vals.second) + std::abs(q_vals.first - q_vals.second));
    return (1 - t) * p + t * q;
}

//DelauneyMesh
//Triangle for 2d
//TetMesh for 3d
//Returns the indices of all of the points of the triangles in the delauney triangulation
// std::vector<std::vector<int>> DelauneyMesh(const Eigen::Matrix2Xf&)
// {
//     
// }

// contourTriMultiDC
/*void contourTriMultiDC(Eigen::Matrix2Xf pts, std::vector<std::vector<int>> tris, std::vector<std::vector<float>> vals)
{
    auto nmat = vals[0].size();
    auto npts = pts.cols();
    auto ntris = tris.size();
    std::vector<std::vector<int>> edgeMap = { {0, 1}, {1, 2}, {2, 0} };

    //Material index at each point
    auto ptMats = vals << std::function(getMaxPos);

    std::map<std::pair<int, int>, int> edgeHash;

    auto edges = table(ntris * 3, std::function([](size_t) {return std::vector<int>(); }));
    auto edgeFaces = table(ntris * 3, std::function([](size_t) {return std::vector<int>(); }));
    auto ct = 0;
    int ind;

    //Make a hashmap of the edges, faces, etc.
    for(int j = 0; j < 3; j++)
    {
        for(int i = 0; i < ntris; i++)
        {
            auto hashIndex = std::pair(
                tris[i][edgeMap[j][1]],
                tris[i][edgeMap[j][1]]
            );
            ind = edgeHash[hashIndex];

            if(ind == 0)
            {
                ct++;
                edges[ct] = edgeMap[j] << std::function([tris, i](const int& index) {return tris[i][index]; });
                edgeFaces[ct] = { i };
                edgeHash[{hashIndex.first, hashIndex.second}] = ct;
                edgeHash[{hashIndex.second, hashIndex.first}] = ct;
                
            }
            else{
                edgeFaces[ind].push_back(i);
            }
        }
    }


}#1#
// perp
// getContourByMat2D
// getContourAllMats2D
// getSectionContours


// getSectionContoursAll
// void getSectionContoursAll(std::vector<Eigen::Matrix3Xf>, std::vector<std::vector<std::vector<float>>>, float);
*/

