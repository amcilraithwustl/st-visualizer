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
Eigen::Vector2f inline getMassPoint(std::vector<Eigen::Vector2f> a)
{
    Eigen::Vector2f temp = Eigen::Vector2f::Zero(2);
    for(auto v : a)
    {
        temp += v;
    }
    return temp/3;
}

// InterpEdge2Mat
// Does a linear interpolation of two materials to produce the midpoint location where the "cross over"
inline Eigen::Vector2f interpEdge2Mat(const Eigen::Vector2f& p, const Eigen::Vector2f& q, std::pair<float, float> p_vals,
                               std::pair<float, float> q_vals)
{
    const float t = std::abs(p_vals.first - p_vals.second) / (std::abs(p_vals.first - p_vals.second) + std::abs(
        q_vals.first - q_vals.second));
    return (1 - t) * p + t * q;
}

// contourTriMultiDC
struct contourTriMultiDCStruct
{
    std::map<int, Eigen::Vector2f> verts;
    std::vector<std::pair<int, int>> segs;
    std::vector<std::pair<int, int>> segMats;
    std::vector<Eigen::Vector2f> fillVerts;
    std::vector<std::vector<int>> fillTris;
    std::vector<int> fillMats;
};

contourTriMultiDCStruct contourTriMultiDC(Eigen::Matrix2Xf pointIndexToPoint, std::vector<std::vector<int>> triangleIndexToCornerIndices,
                                          std::vector<std::vector<float>> pointIndexToMaterialValues);

// perp
inline Eigen::Vector2f perp(Eigen::Vector2f a) { return {-1 * a[1], a[0]}; }

// getContourByMat2D returns new verticies, new segments
inline std::pair<std::vector<Eigen::Vector2f>, std::vector<std::pair<int, int>>> getContourByMat2D(
    std::map<int, Eigen::Vector2f> verts, std::vector<std::pair<int, int>> segs,
    std::vector<std::pair<int, int>> segmats, int mat, float shrink)
{
    //select segments by mat
    std::vector<int> inds1;
    for(size_t i = 0; i < segs.size(); i++) //TODO: This maybe should be based of segmats, not segs
    {
        if(segmats[i].first == mat)
        {
            inds1.push_back(static_cast<int>(i));
        }
    }

    std::vector<int> inds2;
    for(size_t i = 0; i < segs.size(); i++) //TODO: This maybe should be based of segmats, not segs
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

    /*shrink*/
    std::vector<std::pair<int, int>> vertnorms(nverts.size(), {0, 0});

    for(auto seg : nsegs)
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
getContourAllMats2D(const std::map<int, Eigen::Vector2f>& verts, const std::vector<std::pair<int, int>>& segs,
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
        std::vector<std::vector<int>>,
        std::vector<int>,
        std::vector<Eigen::Vector3f>
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
    const auto tris = table(reg.numberoftriangles,
                            std::function([reg](size_t i) { return getTriangleCornerIndices(reg, i); }));
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

    const auto& ftries = res.fillTris;
    const auto& fmats = res.fillMats;
    auto fverts = res.fillVerts << dim2to3;
    return {ctrNewPtsAndSegs, {ftries, fmats, fverts}};
}


// getSectionContoursAll
inline std::pair<std::vector<std::vector<std::pair<std::vector<Eigen::Matrix<float, 3, 1, 0>>, std::vector<std::pair<int
, int>>
>>>, std::vector<std::tuple<std::vector<std::vector<int>>, std::vector<int>, std::vector<Eigen::Matrix<float, 3, 1, 0>>>
>> getSectionContoursAll(std::vector<Eigen::Matrix3Xf> sections,
                         std::vector<std::vector<std::vector<float>>> vals, float shrink)
{
    //TODO: Make sections and vals be input as a single variable to ensure identical length or add length check
    auto contourResult = mapThread(sections, vals, std::function(
                                       [shrink](const Eigen::Matrix3Xf& pts,
                                                const std::vector<std::vector<float>>& vals)
                                       {
                                           return getSectionContours(pts, vals, shrink);
                                       }));

    auto newPointsAndSegs = contourResult << std::function([](
        const std::pair<std::vector<std::pair<std::vector<Eigen::Matrix<float, 3, 1, 0>>, std::vector<std::pair<
                                                  int, int>>>>, std::tuple<
                            std::vector<std::vector<int>>, std::vector<int>, std::vector<Eigen::Matrix<
                                float, 3, 1, 0>>>>& contour)
        {
            return contour.first;
        });

    auto triangleInfo = contourResult << std::function([](
        const std::pair<std::vector<std::pair<std::vector<Eigen::Matrix<float, 3, 1, 0>>, std::vector<std::pair<
                                                  int, int>>>>, std::tuple<
                            std::vector<std::vector<int>>, std::vector<int>, std::vector<Eigen::Matrix<
                                float, 3, 1, 0>>>>& contour)
        {
            return contour.second;
        });

    return { newPointsAndSegs, triangleInfo };
}
