#pragma once
#include <map>
#include <vector>
#include "UtilityFunctions.h"
// orientation (not sure if this should be i or f)
// Returns 1 if left, 0 if none, -1 if right (right hand rule)
int orientation(Eigen::Vector2f a, Eigen::Vector2f b);

// getMaxPos
int getMaxPos(const std::vector<float>& vals);

// getMassPoint
// Might need work based on input type
Eigen::Vector2f inline getMassPoint(Eigen::Matrix2Xf m) { return m.rowwise().mean(); }
Eigen::Vector2f inline getMassPoint(Eigen::Vector2f a, Eigen::Vector2f b, Eigen::Vector2f c) { return (a + b + c) / 3; }

// InterpEdge2Mat
//Might be f or i inputs, but must be f output
Eigen::Vector2f interpEdge2Mat(const Eigen::Vector2f& p, const Eigen::Vector2f& q, std::pair<float, float> p_vals,
                               std::pair<float, float> q_vals)
{
    const float t = std::abs(p_vals.first - p_vals.second) / (std::abs(p_vals.first - p_vals.second) + std::abs(
        q_vals.first - q_vals.second));
    return (1 - t) * p + t * q;
}

// contourTriMultiDC
void contourTriMultiDC(Eigen::Matrix2Xf pts, std::vector<std::vector<int>> tris, std::vector<std::vector<float>> vals)
{
    auto nmat = vals[0].size();
    auto npts = pts.cols();
    auto ntris = tris.size();
    std::vector<std::vector<int>> edgeMap = {{0, 1}, {1, 2}, {2, 0}};

    //Primary Material index at each point
    auto ptMats = vals << std::function(getMaxPos);

    std::map<std::pair<int, int>, int> edgeHash;

    auto edges = table(ntris * 3, std::function([](size_t) { return std::vector<int>(); }));
    auto edgeFaces = table(ntris * 3, std::function([](size_t) { return std::vector<int>(); }));
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
                edges[ct] = edgeMap[j] << std::function([tris, i](const int& index) { return tris[i][index]; });
                edgeFaces[ct] = {i};
                edgeHash[{hashIndex.first, hashIndex.second}] = ct;
                edgeHash[{hashIndex.second, hashIndex.first}] = ct;
            }
            else
            {
                edgeFaces[ind].push_back(i);
            }
        }
    }

    //There should be ct number of elements in the vector now
    //This could be made more efficient by avoiding constructing a new vector
    edges = std::vector(edges.begin(), edges.begin() + ct + 1);
    edgeFaces = std::vector(edgeFaces.begin(), edgeFaces.begin() + ct + 1);


    /*create interpolation points, one per edge with material change*/
    std::map<int, Eigen::Vector2f> edgePoints;
    for(int i = 0; i < edges.size(); i++)
    {
        const auto& pt = edges[i];
        if(ptMats[pt[0]] != ptMats[pt[1]])
        {
            auto& pt0ValIndex = pt[0];
            auto& pt1ValIndex = pt[1];

            auto& pt0PrimaryValueIndex = ptMats[pt0ValIndex];
            auto& pt1PrimaryValueIndex = ptMats[pt1ValIndex];

            auto& vals0 = vals[pt0ValIndex];
            auto& vals1 = vals[pt1ValIndex];

            edgePoints[i] = interpEdge2Mat(
                pts.col(pt[0]),
                pts.col(pt[1]),
                {vals0[pt0PrimaryValueIndex], vals0[pt1PrimaryValueIndex]},
                {vals1[pt0PrimaryValueIndex], vals1[pt1PrimaryValueIndex]}
            );
        }
    }
    std::vector edgePtInds(edges.size(), 0);

    /*create vertices, one per triangle with material change*/
    std::map<int, Eigen::Vector2f> verts;
    ct = 0;
    auto triVertInds = tris << std::function([&](const std::vector<int>& pt)
    {
        if(ptMats[pt[0]] == ptMats[pt[1]] && ptMats[pt[1]] == ptMats[pt[2]])
        {
            return 0;
        }

        ct++;
        const auto a = edgeMap << std::function([&](const std::vector<int>& edge)
        {
            return edgePoints[edgeHash[{pt[edge[0]], pt[edge[1]]}]];
        });
        verts[ct] = getMassPoint(a[0], a[1], a[2]);

        return ct;
    });
    /*create segments*/

    std::vector segs(edges.size(), 0);
    std::vector segMats(edges.size(), std::pair<int, int>());
    int ct2 = 0;
    for(int i = 0; i < edges.size(); i++)
    {
        if(ptMats[edges[i][0]] == ptMats[edges[i][1]])
        {
            ct2++;
            segMats[ct2] = {ptMats[edges[i][0]], ptMats[edges[i][1]]};
            std::pair newSeg;
            if(edgeFaces[i].size() == 1)
            {
                //Boundary edge, connect edge point and triangle point
                ct++;
                verts[ct] = edgePoints[i];
                edgePtInds[i] = ct;
                newSeg = {triVertInds[edgeFaces[i][0]], ct};
            }
            else
            {
                newSeg = {triVertInds[edgeFaces[i][0]], triVertInds[edgeFaces[i][1]]};
            }

            if(orientation(pts.col(edges[i][0]) - pts.col(edges[i][1]), verts[newSeg.first] - pts.col(edges[i][1])) < 0)
            {
                newSeg = {newSeg.second, newSeg.first};
            }

            segs[ct2] = std::move(newSeg);
        }
    }

    //Remove all elements whose indexes are too large
    for(auto& item : verts)
    {
        auto& index = item.first;
        if(index > ct)
        {
            verts.erase(index);
        }
    }

    segs = std::vector(segs.begin(), segs.begin() + 1 + ct2);

    for(auto& item : segMats)
    {
        auto& index = item.first;
        if(index > ct2)
        {
            verts.erase(index);
        }
    }


    /*Create Fill Triangles*/
    std::vector<Eigen::Vector2f> fillVerts;
    fillVerts.reserve(verts.size() + pts.cols()); //We know how big this will be
    for(auto v : verts)
    {
        fillVerts.push_back(v.second);
    }
    for(auto pt : pts.colwise())
    {
        fillVerts.push_back(pt);
    }

    std::vector fillTris(ntris * 6, std::vector<int>());
    std::vector fillMats(ntris * 6, 0);
    auto ct3 = 0;

    /*first type of triangles : dual to mesh edges with a material change*/
    for(int i = 0; i < edges.size(); i++)
    {
        //If the materials on either side of an edge don't match
        if(ptMats[edges[i][0]] != ptMats[edges[i][1]])
        {
            std::pair<int, int> seg;
            if(edgeFaces[i].size() == 1) //if interior edge
            {
                /*boundary edge : connect edge point and triangle point*/
                seg = {triVertInds[edgeFaces[i][0]], edgePtInds[i]};
            }
            else
            {
                /*interior edge : connect two triangle points*/
                seg = {triVertInds[edgeFaces[i][0]], triVertInds[edgeFaces[i][1]]};
            }
            ct3++;
            fillTris[ct3] = std::vector({seg.first, seg.second, ct + edges[i][0]});
            fillMats[ct3] = triVertInds[edgeFaces[i][0]];

            ct3++;
            fillTris[ct3] = std::vector({seg.first, seg.second, ct + edges[i][1]});
            fillMats[ct3] = triVertInds[edgeFaces[i][1]];
        }
    }


    /* second type of triangles: original mesh triangle, if there is no material change,
     or a third of the triangle, if there is some edge with no material change */
    for(int i = 0; i < tris.size(); i++)
    {
        if(ptMats[tris[i][1]] == ptMats[tris[i][2]] && ptMats[tris[i][2]] == ptMats[tris[i][0]])
        {
            ct3 += 1;
            fillTris[ct3] = tris[i] << std::function([ct](int item) { return item + ct; });
            fillMats[ct3] = ptMats[tris[i][0]];
        }
        else
        {
            for(int j = 0; j < 3; j++)
            {
                if(ptMats[tris[i][edgeMap[j][1]]] ==
                    ptMats[tris[i][edgeMap[j][2]]])
                {
                    ct3 += 1;
                    fillTris[ct3] =
                        concat(
                            edgeMap[j] << std::function([i, tris](int in) { return tris[i][in]; }) << std::function(
                                [ct](int item) { return item + ct; })
                            , {triVertInds[i]});
                    fillMats[ct3] = ptMats[tris[i][edgeMap[j][1]]];
                }
            }
        }
    }
}

// perp
inline Eigen::Vector2f perp(Eigen::Vector2f a) { return {-1 * a[1], a[0]}; }
// getContourByMat2D
// getContourAllMats2D
// getSectionContours


// getSectionContoursAll
// void getSectionContoursAll(std::vector<Eigen::Matrix3Xf>, std::vector<std::vector<std::vector<float>>>, float);
