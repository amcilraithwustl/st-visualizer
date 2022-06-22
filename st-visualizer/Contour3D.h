#pragma once
#include "Contour2D.h"
#include "UtilityFunctions.h"

#include "tetgen1.6.0/tetgen.h"

inline int orientation(Eigen::Vector3f u, Eigen::Vector3f v, Eigen::Vector3f w)
{
    auto val = u.cross(v).dot(w);
    if(val < 0) return -1;
    if(val > 0) return 1;
    return 0;
}


template <typename T>
std::vector<T> complement(const std::vector<T>& source, const std::vector<T>& target)
{
    std::vector<T> accumulator;
    accumulator.reserve(source.size());
    for(auto& element : source)
    {
        if(std::find(target.begin(), target.end(), element) == target.end())
        {
            accumulator.push_back(element);
        }
    }
    std::ranges::sort(accumulator);
    return accumulator;
}

inline std::pair<std::vector<int>, std::vector<int>> orderTets(const std::pair<int, int>& edges,
                                                               const std::vector<std::vector<int>>& tets)
{
    std::vector<std::vector<int>> nonEdgeCornersByTet; //This should have two non-edge corners per tet
    {
        nonEdgeCornersByTet.reserve(tets.size());

        for(const auto& tet : tets)
        {                    
            nonEdgeCornersByTet.push_back(complement<int>(tet, { edges.first, edges.second }));
        }
    }

    std::vector<int> nonEdgeCornersSet;
    {
        for(auto& set : nonEdgeCornersByTet)
        {
            nonEdgeCornersSet.insert(nonEdgeCornersSet.end(), set.begin(), set.end());
        }

        std::ranges::sort(nonEdgeCornersSet);
        const auto end = std::ranges::unique(nonEdgeCornersSet).begin();
        nonEdgeCornersSet.resize(end - nonEdgeCornersSet.begin());
    }

    std::vector<int> endpoints; //There will be either 0 or 2 unique corners

     //Which tet, which index in tet
    std::vector tetsByCorner(nonEdgeCornersSet[nonEdgeCornersSet.size() - 1] + 1, std::vector<std::pair<int, int>>());
   
    {
        for(const auto& corner : nonEdgeCornersSet)
        {
            auto& tetEntry = tetsByCorner[corner];
            for(int i = 0; i < nonEdgeCornersByTet.size(); i++)
            {
                for(int j = 0; j < nonEdgeCornersByTet[i].size(); j++)
                {
                    if(nonEdgeCornersByTet[i][j] == corner)
                        tetEntry.emplace_back(i, j);
                }
            }

            if(tetEntry.size() == 1) //If the corner isn't shared by any other tets
            {
                endpoints.push_back(corner);
            }
        }
    }

    std::vector<int> orderedTets;
    int endPoint;
    int nextCorner;
    //If there are no boundary spaces
    if(endpoints.empty())
    {
        //If there are no unique corners, just start with the first tet and go from there
        orderedTets.push_back(0);
        nextCorner = nonEdgeCornersByTet[0][1]; //start with the first tet's second point
        endPoint = nonEdgeCornersByTet[0][0]; //And end with its first point
    }
    //If there are boundary spaces
    else
    {
        //We know the associated set of tets will have exactly one entry
        const auto& temp = tetsByCorner[endpoints[0]][0]; //The tet associated with the first unique corner
        orderedTets.push_back(temp.first);
        nextCorner = nonEdgeCornersByTet[temp.first][1 - temp.second]; //Start with the other corner in that tet
        endPoint = endpoints[1]; //End with the other unique corner
    }

    //Run through all the connected tets to order them correctly
    while(nextCorner != endPoint)
    {

        //TODO: I THINK THERE IS A PROBLEM HERE SOMEWHERE
        auto tetEntry = tetsByCorner[nextCorner];

        //If we have already looked at the first tet in the entry
        if(tetEntry[0].first == orderedTets[orderedTets.size() - 1])
        {
            orderedTets.push_back(tetEntry[1].first); //Push back the second tet index
            nextCorner = nonEdgeCornersByTet[tetEntry[1].first][1 - tetEntry[1].second];
            //The new next is the other corner in that tet
        }
        else
        {
            orderedTets.push_back(tetEntry[0].first); //push back the tet index from the first entry
            nextCorner = nonEdgeCornersByTet[tetEntry[0].first][1 - tetEntry[0].second];
            //The new next is the other corner in that tet
        }
    }
    
    return {orderedTets, endpoints};
}


class lookupTable3D
{
public:
    std::vector<std::vector<std::vector<int>>> table;

    lookupTable3D(size_t size)
    {
        table = std::vector(size, std::vector(size, std::vector(size,-1)));

        //table = std::vector<std::vector<std::vector<int>>>();
        // for(int i = 0 ; i < size; i++)
        // {
        //     auto& entry = table[i];
        //
        //     entry = std::vector(i+1,std::vector<int>());
        //     for(int j = 0; j < i+1; j++)
        //     {
        //         entry[j] = std::vector(j + 1, -1);
        //     }
        // }
    }

    int& at(size_t a, size_t b, size_t c)
    {
        //Order arguments
        if(b>a)
        {
            std::swap(a, b);
        }
        if(c>a)
        {
            std::swap(c, a);
        }
        if(c>b)
        {
            std::swap(b, c);
        }
        return table[a][b][c];
    }
};

inline std::tuple<std::vector<Eigen::Matrix<float, 3, 1, 0>>, std::vector<std::vector<int>>, std::vector<std::pair<
                      int, int>>> contourTetMultiDC(const std::vector<Eigen::Vector3f>& pts,
                                                    const std::vector<std::vector<int>>& tets,
                                                    std::vector<std::vector<float>> vals)
{
    auto nmat = vals[0].size();
    auto npts = pts.size();
    auto ntets = tets.size();
    std::vector<std::pair<int, int>> edgeMap = {{0, 1}, {0, 2}, {0, 3}, {1, 2}, {1, 3}, {2, 3}};

    //get material index at each point
    std::vector<int> ptMats;
    {
        ptMats.reserve(npts);
        for(int i = 0; i < pts.size(); i++)
        {
            ptMats.push_back(getMaxPos(vals[i]));
        }
    }

    //create adj table from edges to faces
    std::vector edgeHash(npts, std::vector(npts, -1));
    std::vector<std::pair<int, int>> edges;
    std::vector<std::vector<int>> edgeTets;
    {
        edges.reserve(ntets * 6);
        edgeTets.reserve(ntets * 6);
        const int temp = ntets % 10 == 0
                         ? ntets / 10
                         : (ntets + 10) / 10;
        for(int i = 0; i < ntets; i++)
        {
            for(int j = 0; j < 6; j++)
            {
                if(i % temp == 0 && j == 0) std::cout << static_cast<float>(i) / ntets * 100 << "%" << std::endl;
                const auto edge = edgeMap[j];
                const auto& tet = tets[i];
                std::pair endpoints = {tet[edge.first], tet[edge.second]};
                auto ind = edgeHash[endpoints.first][endpoints.second];
                if(ind == -1)
                {
                    edges.push_back(endpoints);
                    edgeTets.push_back({i});
                    edgeHash[endpoints.first][endpoints.second] = edges.size() - 1;
                    edgeHash[endpoints.second][endpoints.first] = edges.size() - 1;
                }
                else
                {
                    edgeTets[ind].push_back(i);
                }
            }
        }
        std::cout << "Adj table DONE." << std::endl;
    }

    //create interpolation points, one per edge with material change
    std::vector<std::pair<Eigen::Vector3f, bool>> edgePoints;
    {
        edgePoints.reserve(edges.size());
        for(auto& edge : edges)
        {
            const auto& pMatIndex = ptMats[edge.first];
            const auto& qMatIndex = ptMats[edge.second];
            if(pMatIndex == qMatIndex)
            {
                edgePoints.push_back({{0, 0, 0}, false});
            }
            else
            {
                const auto& p = pts[edge.first];
                const auto& q = pts[edge.second];
                const auto& pvals = vals[edge.first];
                const auto& qvals = vals[edge.second];

                edgePoints.emplace_back(interpEdge2Mat<3>(
                                            p, q,
                                            {pvals[pMatIndex], pvals[qMatIndex]},
                                            {qvals[pMatIndex], qvals[qMatIndex]}
                                        ), true
                );
            }
        }
    }
    std::cout << "Adj table DONE." << std::endl;

    //create vertices, one per tet with material change
    std::vector<Eigen::Vector3f> verts;
    std::vector<int> tetVertInds;
    {
        verts.reserve(ntets * 5);
        tetVertInds.reserve(ntets);
        for(const auto& tet : tets)
        {
            if(ptMats[tet[0]] == ptMats[tet[1]] && ptMats[tet[0]] == ptMats[tet[2]] && ptMats[tet[0]] == ptMats[tet[3]])
            {
                tetVertInds.push_back(-1);
            }
            else
            {
                std::vector<Eigen::Vector3f> temp;
                for(const auto& edge : edgeMap)
                {
                    const auto& point = edgePoints[edgeHash[tet[edge.first]][tet[edge.second]]];
                    if(point.second) temp.push_back(point.first);
                }
                verts.push_back(getMassPoint<3>(temp));
                tetVertInds.push_back(verts.size() - 1);
            }
        }
        std::cout << "Tet points DONE." << std::endl;
    }

    //Create Segments

    //TODO: Extremely slow. Reduce size of cube vector and Consider a faster allocated option.
    lookupTable3D bdFaceHash(edges.size());
    std::vector<std::vector<int>> segs;
    std::vector<std::pair<int, int>> segMats;
    {
        segs.reserve(edges.size());
        segMats.reserve(edges.size());
        const int temp = edges.size() % 10 == 0
                         ? edges.size() / 10
                         : (edges.size() / 10) + 1;

        for(int i = 0; i < edges.size(); i++)
        {
            if(i % temp == 0) std::cout << static_cast<float>(i) / edges.size() * 100 << "%" << std::endl;
            const auto& edge = edges[i];

            if(ptMats[edge.first] != ptMats[edge.second])
            {
                segMats.emplace_back(ptMats[edge.first], ptMats[edge.second]);
                auto [ot, ends] = orderTets(edges[i], subset(tets, edgeTets[i]));

                std::vector<int> newseg;
                if(ends.size() > 0)
                {
                    //if there are boundary faces
                    std::vector ps = {-1, -1};
                    for(auto j = 0; j < ps.size(); j++)
                    {
                        std::vector tri = {edge.first, edge.second, ends[j]};
                        std::ranges::sort(tri);


                        // if (!bdFaceHash.contains({ tri[0], tri[1], tri[2] })) {
                        //     bdFaceHash.insert({ { tri[0], tri[1], tri[2] } , -1 });
                        // }
                        auto& hashValue = bdFaceHash.at(tri[0],tri[1],tri[2]);
                        ps[j] = hashValue;
                        if(hashValue == -1)
                        {
                            const std::vector<std::pair<int, int>> triangleEdges = {{0, 1}, {1, 2}, {2, 0}};

                            //Add Face Vertex
                            std::vector<Eigen::Vector3f> massedPoints;
                            for(const auto& triangleEdgeEndpoints : triangleEdges)
                            {
                                auto index = edgeHash[tri[triangleEdgeEndpoints.first]][tri[triangleEdgeEndpoints.second]];
                                if(edgePoints[index].second) 
                                    massedPoints.push_back(edgePoints[index].first);
                            }
                            verts.push_back(getMassPoint<3>(massedPoints));
                            ps[j] = verts.size() - 1;
                            hashValue = ps[j];
                        }
                    }
                    auto t = subset(tetVertInds, subset(edgeTets[i], ot));
                    newseg = concat(concat({ps[0]}, t), {ps[1]});
                }
                else
                {
                    //If no boundary faces
                    //TODO: This could probably be done in one go without the else
                    newseg = subset(tetVertInds, subset(edgeTets[i], ot));
                }

                //orient the polygon
                int p1, p2;
                if(ot.size() == 1)
                {
                    p1 = ends[0];
                    p2 = ends[1];
                }
                else
                {
                    p1 = complement(tets[edgeTets[i][ot[0]]], tets[edgeTets[i][ot[1]]])[0];
                    p2 = complement(tets[edgeTets[i][ot[0]]], {edge.first, edge.second, p1})[0];
                }

                if(orientation(pts[edge.first] - pts[edge.second], pts[p1] - pts[edge.first], pts[p2] - pts[p1]) < 0)
                {
                    std::ranges::reverse(newseg);
                }

                segs.push_back(newseg);
            }
        }
    }
    return {verts, segs, segMats};
}


inline void tetralizeMatrix(const Eigen::Matrix3Xf& pts, tetgenio& out)
{
    tetgenio in;

    in.firstnumber = 0; //Arrays start at 0

    in.numberofpoints = static_cast<int>(pts.cols());
    in.pointlist = new REAL[in.numberofpoints * 3];
    for(int i = 0; i < in.numberofpoints; i++)
    {
        for(int j = 0; j < 3; j++)
        {
            in.pointlist[3 * i + j] = static_cast<double>(pts.col(i)(j));
        }
    }

    tetrahedralize(_strdup(
                       "z" //Start arrays at zero
                       "V" //Verbose for debugging
                       ""
                       // "O0" //Level of mesh optimization (none)
                       // "S0" //Max number of added points (none)
                   ), &in, &out);
}

inline Eigen::Vector3f getFaceNorm(const std::vector<Eigen::Vector3f>& pts)
{
    Eigen::Vector3f sum = {0, 0, 0};
    for(int i = 0; i < pts.size(); i++)
    {
        const auto offset = (i + 1) % pts.size();
        sum += pts[i].cross(pts[offset]);
    }
    return sum;
}

inline std::pair<std::vector<Eigen::Vector3f>, std::vector<std::vector<int>>> getContourByMat3D(
    const std::vector<Eigen::Vector3f>& verts,
    const std::vector<std::vector<int>>& segs,
    const std::vector<std::pair<int, int>>& segmats,
    int mat,
    float shrink)
{
    //Select segments by target material
    std::vector<int> inds1;
    std::vector<int> inds2;
    {
        for(int i = 0; i < segs.size(); i++)
        {
            if(segmats[i].first == mat) inds1.push_back(i);
            if(segmats[i].second == mat) inds2.push_back(i);
        }
    }

    auto reversedInds = subset(segs, inds2);
    for(auto& seg : reversedInds)
    {
        std::ranges::reverse(seg);
    }
    auto nsegs = concat(subset(segs, inds1), reversedInds);//All these segements start with the target material

    //Prune unused vertices
    std::vector vertUsed(verts.size(), false);
    {
        for(const auto& seg : nsegs)
        {
            for(const auto& ind : seg)
            {
                vertUsed[ind] = true;
            }
        }
    }

    std::vector<int> nVertInds;//Indices of the vertices we care about
    for(int i = 0; i < vertUsed.size(); i++)
    {
        if(vertUsed[i] == true) nVertInds.push_back(i);
    }

    std::vector vertNewInds(verts.size(), -1);//Map the old indices to the new indices
    for(int i = 0; i < nVertInds.size(); i++)
    {
        vertNewInds[nVertInds[i]] = i;
    }
    auto nverts = subset(verts, nVertInds);//New vertices (can be mapped)
    std::vector<std::vector<int>> nsegs2;
    {
        for(const auto& seg : nsegs)
        {
            nsegs2.push_back({});
            for(const auto& pt : seg)
            {
                nsegs2[nsegs2.size() - 1].push_back(vertNewInds[pt]);
            }
        }
    }

    //shrink
    std::vector<Eigen::Vector3f> vertNorms(nverts.size(), {0, 0, 0});
    for(const auto& seg : nsegs2)
    {
        std::vector<Eigen::Vector3f> points = subset(nverts, seg);
        auto nm = getFaceNorm(points);
        for(auto index : seg)
        {
            vertNorms[index] += nm;
        }
    }

    for(int i = 0; i < vertNorms.size(); i++)
    {
        const auto faceVector = vertNorms[i];
        const auto normalized = faceVector.normalized();
        nverts[i] += shrink * normalized;
    }

    return {nverts, nsegs2};
}

inline std::vector<std::pair<std::vector<Eigen::Vector3f>, std::vector<std::vector<int>>>> getContourAllMats3D(
    const std::vector<Eigen::Vector3f>& verts,
    const std::vector<std::vector<int>>& segs,
    const std::vector<std::pair<int, int>>& segmats,
    const int& nmats,
    const float& shrink)
{
    std::vector<std::pair<std::vector<Eigen::Vector3f>, std::vector<std::vector<int>>>> a;
    for(int i = 0; i < nmats; i++)
    {
        a.push_back(getContourByMat3D(verts, segs, segmats, i, shrink));
    }
    return a;
}
