#pragma once
#include <map>
#include <vector>
#include "UtilityFunctions.h"
// orientation (not sure if this should be i or f)
// Returns 1 if left, 0 if none, -1 if right (right hand rule)
int orientation(Eigen::Vector2f a, Eigen::Vector2f b);

// getMaxPos
int getMaxPos(const std::vector<float>& material_values);

// getMassPoint
// Might need work based on input type
template <unsigned int N>
Eigen::Matrix<float, N, 1> getMassPoint(const std::vector<Eigen::Matrix<float, N, 1>>& a)
{
    Eigen::Matrix<float, N, 1> temp = Eigen::Matrix<float, N, 1>::Zero(N);
    for(const Eigen::Matrix<float, N, 1>& v : a)
    {
        temp += v;
    }
    return temp / a.size();
}

// InterpEdge2Mat
// Does a linear interpolation of two materials to produce the midpoint location where the "cross over"
template <unsigned int N>
Eigen::Matrix<float, N, 1> interpEdge2Mat(const Eigen::Matrix<float, N, 1>& p, const Eigen::Matrix<float, N, 1>& q,
                                          const std::pair<float, float>& p_vals,
                                          const std::pair<float, float>& q_vals)
{
    const auto top = std::abs(p_vals.first - p_vals.second);
    const auto bottom = std::abs(p_vals.first - p_vals.second) + std::abs(q_vals.first - q_vals.second);
    const float t = top / bottom;
    return p * (1 - t) + q * t;
}

// contourTriMultiDC
struct contourTriMultiDCStruct
{
    std::vector<Eigen::Vector2f> verts;
    std::vector<std::pair<int, int>> segs;
    std::vector<std::pair<int, int>> segMats;
    std::vector<Eigen::Vector2f> fillVerts;
    std::vector<std::vector<int>> fillTris;
    std::vector<int> fillMats;
};

contourTriMultiDCStruct contourTriMultiDC(const Eigen::Matrix2Xf& pointIndexToPoint,
                                          const std::vector<std::vector<int>>& triangleIndexToCornerIndices,
                                          const std::vector<std::vector<float>>& pointIndexToMaterialValues);

// perp
inline Eigen::Vector2f perp(Eigen::Vector2f a) { return {-1 * a[1], a[0]}; }

// getContourByMat2D returns new verticies, new segments
inline std::pair<std::vector<Eigen::Vector2f>, std::vector<std::pair<int, int>>> getContourByMat2D(
    std::vector<Eigen::Vector2f> verts, std::vector<std::pair<int, int>> segs,
    std::vector<std::pair<int, int>> segmats, int mat, float shrink)
{
    //select segments by mat
    std::vector<int> inds1;
    for(size_t i = 0; i < segmats.size(); i++) //TODO: This maybe should be based of segmats, not segs
    {
        if(segmats[i].first == mat)
        {
            inds1.push_back(static_cast<int>(i));
        }
    }

    std::vector<int> inds2;
    for(size_t i = 0; i < segmats.size(); i++) //TODO: This maybe should be based of segmats, not segs
    {
        if(segmats[i].second == mat)
        {
            inds2.push_back(static_cast<int>(i));
        }
    }

    auto nsegs = concat(
        subset(segs, inds1),
        subset(segs, inds2) << std::function([](std::pair<int, int> i) { return std::pair(i.second, i.first); })
    );

    /*prune unused vertices*/
    std::vector vertsUsed(verts.size(), false);
    for(auto& seg : nsegs)
    {
        vertsUsed[seg.first] = true;
        vertsUsed[seg.second] = true;
    }

    std::vector<size_t> nvertInds;
    for(size_t i = 0; i < vertsUsed.size(); i++) { if(vertsUsed[i]) nvertInds.push_back(i); }

    std::vector<size_t> vertNewInds(verts.size(), 0);
    for(size_t i = 0; i < nvertInds.size(); i++)
    {
        vertNewInds[nvertInds[i]] = i;
    }

    auto nverts = nvertInds << std::function([verts](const size_t& i) { return verts.at(static_cast<int>(i)); });
    std::vector<std::pair<size_t, size_t>> adjusted_nsegs;
    {
        adjusted_nsegs.reserve(nsegs.size());
        for(auto& seg : nsegs)
        {
            adjusted_nsegs.emplace_back(vertNewInds[seg.first], vertNewInds[seg.second]);
        }
    }
    /*shrink*/
    std::vector<std::pair<int, int>> vertnorms(nverts.size(), {0, 0});
    for(auto& seg : adjusted_nsegs)
    {
        auto nm = perp(nverts[seg.second] - nverts[seg.first]);
        vertnorms[seg.first].first += nm(0);
        vertnorms[seg.first].second += nm(1);

        vertnorms[seg.second].first += nm(0);
        vertnorms[seg.second].second += nm(1);
    }

    nverts = nverts << std::function([vertnorms, shrink](const Eigen::Vector2f& pt, size_t i)
    {
        return (pt + shrink * Eigen::Vector2i(vertnorms[i].first, vertnorms[i].second).cast<float>()).eval();
    });

    return {nverts, nsegs};
}

// getContourAllMats2D each index is new vertices, new segments
inline std::vector<std::pair<std::vector<Eigen::Matrix<float, 2, 1, 0>>, std::vector<std::pair<int, int>>>>
getContourAllMats2D(const std::vector<Eigen::Vector2f>& verts, const std::vector<std::pair<int, int>>& segs,
                    const std::vector<std::pair<int, int>>& segmats, const int& nmat, const float& shrink)
{
    return table(
        nmat, std::function([&](size_t i)
            {
                return getContourByMat2D(verts, segs, segmats, static_cast<int>(i), shrink);
            }
        ));
}

// getSectionContours
inline
std::pair<
    std::vector<
        std::pair<
            std::vector<Eigen::Vector3f>,
            std::vector<std::pair<int, int>>
        >
    >,
    std::tuple<
        std::vector<Eigen::Vector3f>,
        std::vector<std::vector<int>>,
        std::vector<int>

    >
>
getSectionContours(
    Eigen::Matrix3Xf pts, std::vector<std::vector<float>> vals, float shrink)
{
    int nmat = vals[0].size();
    float z = pts.col(0)(2);
    Eigen::Matrix2Xf npts(2, pts.cols());
    for(int i = 0; i < pts.cols(); i++)
    {
        npts.col(i) = Eigen::Vector2f({pts.col(i)(0), pts.col(i)(1)});
    }

    auto reg = triangulateMatrix(npts);
    auto tris = table(static_cast<size_t>(reg.numberoftriangles), std::function([reg](size_t i)
    {
        return getTriangleCornerIndices(reg, i);
    }));

    auto res = contourTriMultiDC(npts, tris, vals);

    auto ctrs = getContourAllMats2D(res.verts, res.segs, res.segMats, nmat, shrink);

    auto dim2to3 = std::function([z](Eigen::Vector2f pt) { return Eigen::Vector3f({pt(0), pt(1), z}); });

    auto ctrNewPtsAndSegs = ctrs << std::function(
        [dim2to3](std::pair<std::vector<Eigen::Vector2f>, std::vector<std::pair<int, int>>> ctr)
        {
            const auto& newVertices = ctr.first;
            const auto& newSegments = ctr.second;
            return std::pair(
                newVertices << dim2to3,
                newSegments); //Do I need to deep copy the segments?
        });

    const auto& ftris = res.fillTris;
    const auto& fmats = res.fillMats;
    auto fverts = res.fillVerts << dim2to3;
    return {ctrNewPtsAndSegs, {fverts, ftris, fmats}};
}


// getSectionContoursAll
inline std::pair<std::vector<std::vector<std::pair<std::vector<Eigen::Matrix<float, 3, 1, 0>>, std::vector<std::pair<
                                                       int, int>>>>>, std::vector<std::tuple<
                     std::vector<Eigen::Matrix<float, 3, 1, 0>>, std::vector<std::vector<int>>, std::vector<int>>>>
getSectionContoursAll(std::vector<Eigen::Matrix3Xf> sections,
                      std::vector<std::vector<std::vector<float>>> vals, float shrink)
{
    std::vector<std::vector<std::pair<std::vector<Eigen::Vector3f>, std::vector<std::pair<int, int>>>>>
        newPointsAndSegs;
    newPointsAndSegs.reserve(sections.size());

    std::vector<std::tuple<std::vector<Eigen::Vector3f>, std::vector<std::vector<int>>, std::vector<int>>> triangleInfo;
    triangleInfo.reserve(sections.size());

    for(int i = 0; i < sections.size(); i++)
    {
        auto pts = sections[i];
        auto v = vals[i];


        auto contour = getSectionContours(pts, v, shrink);
        newPointsAndSegs.push_back(std::move(contour.first));
        triangleInfo.push_back(std::move(contour.second));
    }
    return {newPointsAndSegs, triangleInfo};
}
