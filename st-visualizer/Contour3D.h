#pragma once
#include "Contour2D.h"
#include "UtilityFunctions.h"
#include <iomanip>
#include "tetgen1.6.0/tetgen.h"

#define LOADING_SIZE 50
#define UNLOADED_SYMBOL char(176)
#define LOADED_SYMBOL char(219)
#define DEBUG true


//1) Generate vertices, don't append, create flat array w/ fixed size
//2) Remove 2d/3d vector to hashmaps
//3) Store indices of the other two vertices not on the edge
//4) Figure out how to reduce number of data structures

inline int orientation(Eigen::Vector3f u, Eigen::Vector3f v, Eigen::Vector3f w)
{
    auto val = u.cross(v).dot(w);
    if(val < 0) return -1;
    if(val > 0) return 1;
    return 0;
}


//O(nlog(n)+mlog(m) operation, but every usage n<=m<=4, so probably not a problem
inline std::vector<int> complement(std::vector<int> source, std::vector<int> target)
{
    std::ranges::sort(source);
    std::ranges::sort(target);

    std::vector<int> accumulator;
    accumulator.reserve(source.size());
    int i = 0, j = 0;
    while(i < source.size() && j < target.size())
    {
        auto& element = source[i];
        //If the element is ahead of the target comparison, there are no more matches with it
        if(element > target[j])
        {
            j++;
        }
        //If the element is behind the target comparison, we can safely use it
        else if(element < target[j])
        {
            accumulator.push_back(element);
            i++;
        }
        //If the element is equal, we know it cannot be in the set and we skip it
        else if(element == target[j])
        {
            i++;
        }
    }

    //Push back any remaining points
    for(; i < source.size(); i++)
    {
        accumulator.push_back(source[i]);
    }
    return accumulator;
}

//TODO: This function is what takes the longest?
inline std::pair<std::vector<int>, std::vector<int>> orderTets(const std::pair<int, int>& edges,
                                                               const std::vector<std::vector<int>>& tets)
{
    std::pair<std::vector<int>, std::vector<int>> ret;
    auto& orderedTets = ret.first;
    orderedTets.reserve(tets.size());

    //Which 2 other corners does each tet have?
    std::vector<std::vector<int>> cornersByTet; //This should have two non-edge vertices per tet
    {
        cornersByTet.reserve(tets.size());
        std::vector edgeEndpoints = { edges.first, edges.second };
        for(const auto& tet : tets)
        {
            cornersByTet.push_back(complement(tet, edgeEndpoints));
        }
    }

    

    //Which tet, which index in tet
    std::unordered_map<int, std::vector<std::pair<int, int>>> tetsByCorner;
    {

        //For Each tet
        //O(2n) b/c each tet has exactly two stored corners
        for (int tetIndex = 0; tetIndex < cornersByTet.size(); tetIndex++)//O(n)
        {
            //For each of the two corners in that tet
            for (int cornerIndex = 0; cornerIndex < cornersByTet[tetIndex].size(); cornerIndex++) //O(2)
            {
                //Push back the corners that it touches
                const auto& corner = cornersByTet[tetIndex][cornerIndex];
                tetsByCorner[corner].emplace_back(tetIndex, cornerIndex);
            }
        }
        
        //
        // //For each corner (value is index of corner in pts)
        // for(const auto& corner : cornerSet)
        // {
        //     //Find each tet it touches
        //     auto& tetEntry = tetsByCorner[corner];
        //     tetEntry.reserve(tets.size());
        //     for(int tetIndex = 0; tetIndex < cornersByTet.size(); tetIndex++)//O(n)
        //     {
        //         for(int cornerIndex = 0; cornerIndex < cornersByTet[tetIndex].size(); cornerIndex++) //O(1) b/c n==2
        //         {
        //             if(cornersByTet[tetIndex][cornerIndex] == corner)
        //                 tetEntry.emplace_back(tetIndex, cornerIndex);
        //         }
        //     }
        // }

        
    }

    //Set of all the corners, sorted
    //O(nlogn)
    std::vector<int> cornerSet;
    {
        cornerSet.reserve(cornersByTet.size() * 2);
        for (auto& set : cornersByTet)
        {
            cornerSet.insert(cornerSet.end(), set.begin(), set.end());
        }

        std::ranges::sort(cornerSet);
        const auto end = std::ranges::unique(cornerSet).begin();
        cornerSet.resize(end - cornerSet.begin());
    }

    //Find the two endpoints (if they exist)
    auto& endpoints = ret.second;
    {
        for (const auto& corner : cornerSet)
        {
            //Find each tet it touches
            auto& tetEntry = tetsByCorner[corner];

            if (tetEntry.size() == 1) //If the corner isn't shared by any other tets
            {
                endpoints.push_back(corner);//Then save it as an endpoint
            }
            if (endpoints.size() == 2) break;//We know there will be no more than 2
        }
    }

    int endPoint;
    int nextCorner;
    //If there are no boundary spaces
    if(endpoints.empty())
    {
        //If there are no unique corners, just start with the first tet and go from there
        orderedTets.push_back(0);
        nextCorner = cornersByTet[0][1]; //start with the first tet's second point
        endPoint = cornersByTet[0][0]; //And end with its first point
    }
    //If there are boundary spaces
    else
    {
        //We know the associated set of tets will have exactly one entry
        const auto& temp = tetsByCorner[endpoints[0]][0]; //The tet associated with the first unique corner
        orderedTets.push_back(temp.first);
        nextCorner = cornersByTet[temp.first][1 - temp.second]; //Start with the other corner in that tet
        endPoint = endpoints[1]; //End with the other unique corner
    }

    //Run through all the connected tets to order them correctly
    //Stop when we get to the endpoint (Which will be on the first tet we started at or on the other unconnected tet)
    while (nextCorner != endPoint)//O(n)
    {
        const auto& tetEntry = tetsByCorner[nextCorner];

        //If we have already looked at the first tet in the entry, then we know it is the second and vice versa
        //First is tet index, second is the corner's index in that tet (0 or 1 b/c we removed the two we are rotating around)
        const auto& selectedTet = tetEntry[0].first == orderedTets[orderedTets.size() - 1]
            ? tetEntry[1]
            : tetEntry[0];

        orderedTets.push_back(selectedTet.first); //Push back the tet index
        nextCorner = cornersByTet[selectedTet.first][1 - selectedTet.second];//Save the other corner from that tet
    }

    return ret;
}


class Hash3d
{
public:
    // std::vector<std::vector<std::vector<int>>> table;
    std::unordered_map<int, std::unordered_map<int, std::unordered_map<int, int>>> table;
    // std::map<std::pair<size_t, size_t>, std::vector<int>> table;
    size_t size;

    Hash3d(size_t size) : size(size)
    {
        // table = std::vector(size, std::vector(size, std::vector(size,-1)));
        // table.reserve(size);
    }

    int& at(size_t a, size_t b, size_t c)
    {
        //Order arguments
        if(b > a)
        {
            std::swap(a, b);
        }
        if(c > a)
        {
            std::swap(c, a);
        }
        if(c > b)
        {
            std::swap(b, c);
        }

        if(!table[a][b].contains(c))
        {
            table[a][b][c] = -1;
        }
        return table[a][b][c];
    }
};

struct Hash2d
{
    std::unordered_map<int, std::unordered_map<int, int>> table;

    int& at(int a, int b)
    {
        if(a > b)
        {
            std::swap(a, b);
        }
        if(!table[a].contains(b))
        {
            table[a][b] = -1;
        }
        return table[a][b];
    }
};



inline std::tuple<std::vector<Eigen::Matrix<float, 3, 1, 0>>, std::vector<std::vector<int>>, std::vector<std::pair<
                      int, int>>> contourTetMultiDC(const std::vector<Eigen::Vector3f>& points_by_index,
                                                    const std::vector<std::vector<int>>& tets_by_index,
                                                    std::vector<std::vector<float>> vals_by_point_index)
{
    std::cout << std::fixed;
    std::cout << std::setprecision(0);
    auto number_of_materials = vals_by_point_index[0].size();
    auto number_of_points = points_by_index.size();
    auto number_of_tets = tets_by_index.size();
    std::vector<std::pair<int, int>> corner_combinations = {{0, 1}, {0, 2}, {0, 3}, {1, 2}, {1, 3}, {2, 3}};

    //get material index at each point
    std::vector<int> primary_material_by_point_index;
    {
        primary_material_by_point_index.reserve(number_of_points);
        for(size_t point_index = 0; point_index < points_by_index.size(); point_index++)
        {
            primary_material_by_point_index.push_back(getMaxPos(vals_by_point_index[point_index]));
        }
    }

    //create adj table from edges to faces


    Hash2d edge_index_by_endpoint_indices;
    std::vector<std::pair<int, int>> edges_by_index;
    std::vector<std::vector<int>> tets_by_edge_index;
    {
        edges_by_index.reserve(number_of_tets * 6);
        tets_by_edge_index.reserve(number_of_tets * 6);
        const int display_fraction = number_of_tets % LOADING_SIZE == 0
                         ? number_of_tets / LOADING_SIZE
                         : (number_of_tets / LOADING_SIZE) + 1;
        for (size_t tet_index = 0; tet_index < number_of_tets; tet_index++)
        {
#if DEBUG
            if (tet_index % display_fraction == 0)
                std::cout << std::string(tet_index / display_fraction, LOADED_SYMBOL) + std::string(
                    LOADING_SIZE - (tet_index / display_fraction), UNLOADED_SYMBOL) << "\r";
#endif
            for (int combination_index = 0; combination_index < 6; combination_index++)
            {

                const auto& corner_combination = corner_combinations[combination_index];
                const auto& tet = tets_by_index[tet_index];
                const auto& firstEndpoint = tet[corner_combination.first];
                const auto& secondEndpoint = tet[corner_combination.second];
                auto& edge_index = edge_index_by_endpoint_indices.at(firstEndpoint, secondEndpoint);
                if (edge_index == -1) //If the edge does not exist
                {
                    edges_by_index.emplace_back(firstEndpoint, secondEndpoint);
                    tets_by_edge_index.push_back({ static_cast<int>(tet_index) });
                    edge_index = edges_by_index.size() - 1;
                }
                else
                {
                    tets_by_edge_index[edge_index].push_back(tet_index);
                }
            }
        }
        std::cout << "Adj table DONE." << std::endl;
    }

    //create interpolation points_by_index, one per edge with material change
    std::unordered_map<int, Eigen::Vector3f> edgePoint_by_edge_index;
    {
        edgePoint_by_edge_index.reserve(edges_by_index.size());
        for(int i = 0; i < edges_by_index.size(); i++)
        {
            auto& edge = edges_by_index[i];
            const auto& p_material_index = primary_material_by_point_index[edge.first];
            const auto& q_material_index = primary_material_by_point_index[edge.second];

            if(p_material_index != q_material_index)
            {
                const auto& p = points_by_index[edge.first];
                const auto& q = points_by_index[edge.second];
                const auto& pvals = vals_by_point_index[edge.first];
                const auto& qvals = vals_by_point_index[edge.second];

                edgePoint_by_edge_index[i] = interpEdge2Mat<3>(
                    p, q,
                    {pvals[p_material_index], pvals[q_material_index]},
                    {qvals[p_material_index], qvals[q_material_index]}
                );
            }
        }
    }
    std::cout << "Adj table DONE." << std::endl;

    //create vertices, one per tet with material change
    std::vector<Eigen::Vector3f> vertex_by_index;
    std::vector<int> vertex_index_by_tet_index;
    {
        vertex_by_index.reserve(number_of_tets * 5);
        vertex_index_by_tet_index.reserve(number_of_tets);
        for(const auto& tet : tets_by_index)
        {
            //If there is no material change within the tet
            if(primary_material_by_point_index[tet[0]] == primary_material_by_point_index[tet[1]]
                && primary_material_by_point_index[tet[0]] == primary_material_by_point_index[tet[2]]
                && primary_material_by_point_index[tet[0]] == primary_material_by_point_index[tet[3]])
            {
                vertex_index_by_tet_index.push_back(-1);//There is no central vertex
            }
            else
            {
                std::vector<Eigen::Vector3f> temp;
                temp.reserve(corner_combinations.size());
                for(const auto& corner_combination : corner_combinations)
                {
                    const auto pointIndex = edge_index_by_endpoint_indices.at(
                        tet[corner_combination.first], tet[corner_combination.second]);

                    if(edgePoint_by_edge_index.contains(pointIndex)) temp.push_back(edgePoint_by_edge_index[pointIndex]);
                }
                vertex_by_index.push_back(getMassPoint<3>(temp));
                vertex_index_by_tet_index.push_back(vertex_by_index.size() - 1);
            }
        }
        std::cout << "Tet points_by_index DONE." << std::endl;
    }

    //Create Segments

    Hash3d bdFaceHash(edges_by_index.size());
    std::vector<std::vector<int>> segs;
    std::vector<std::pair<int, int>> segMats;
    {
        segs.reserve(edges_by_index.size());
        segMats.reserve(edges_by_index.size());
        const int temp = edges_by_index.size() % LOADING_SIZE == 0
                         ? edges_by_index.size() / LOADING_SIZE
                         : (edges_by_index.size() / LOADING_SIZE) + 1;

        for(int i = 0; i < edges_by_index.size(); i++)
        {
#if DEBUG
            if(i % temp == 0)
                std::cout << std::string(i / temp, LOADED_SYMBOL) + std::string(
                    LOADING_SIZE - (i / temp), UNLOADED_SYMBOL) << "\r";
#endif
            const auto& edge = edges_by_index[i];

            if(primary_material_by_point_index[edge.first] != primary_material_by_point_index[edge.second])
            {
                segMats.emplace_back(primary_material_by_point_index[edge.first],
                                     primary_material_by_point_index[edge.second]);
                auto [ot, ends] = orderTets(edges_by_index[i], subset(tets_by_index, tets_by_edge_index[i]));

                std::vector<int> newseg = subset(vertex_index_by_tet_index, subset(tets_by_edge_index[i], ot));
                if(ends.size() > 0)
                {
                    //if there are boundary faces
                    std::vector ps = {-1, -1};
                    for(auto j = 0; j < ps.size(); j++)
                    {
                        std::vector tri = {edge.first, edge.second, ends[j]};
                        std::ranges::sort(tri);

                        auto& hashValue = bdFaceHash.at(tri[0], tri[1], tri[2]);
                        ps[j] = hashValue;
                        if(hashValue == -1)
                        {
                            const std::vector<std::pair<int, int>> triangleEdges = {{0, 1}, {1, 2}, {2, 0}};

                            //Add Face Vertex
                            std::vector<Eigen::Vector3f> massedPoints;
                            for(const auto& triangleEdgeEndpoints : triangleEdges)
                            {
                                auto index = edge_index_by_endpoint_indices.at(
                                    tri[triangleEdgeEndpoints.first], tri[triangleEdgeEndpoints.second]);
                                if(edgePoint_by_edge_index.contains(index))
                                    massedPoints.push_back(edgePoint_by_edge_index[index]);
                            }
                            vertex_by_index.push_back(getMassPoint<3>(massedPoints));
                            ps[j] = vertex_by_index.size() - 1;
                            hashValue = ps[j];
                        }
                    }
                    
                    newseg = concat(concat({ps[0]}, newseg), {ps[1]});
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
                    p1 = complement(tets_by_index[tets_by_edge_index[i][ot[0]]],
                                    tets_by_index[tets_by_edge_index[i][ot[1]]])[0];
                    p2 = complement(tets_by_index[tets_by_edge_index[i][ot[0]]], {edge.first, edge.second, p1})[0];
                }

                if(orientation(points_by_index[edge.first] - points_by_index[edge.second],
                               points_by_index[p1] - points_by_index[edge.first],
                               points_by_index[p2] - points_by_index[p1]) < 0)
                {
                    std::ranges::reverse(newseg);
                }

                segs.push_back(newseg);
            }
        }
    }
    return {vertex_by_index, segs, segMats};
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
    auto nsegs = concat(subset(segs, inds1), reversedInds); //All these segements start with the target material

    std::vector<int> nVertInds; //Indices of the vertices we care about
    //TODO: This process is slow. Could be sped up w/ std functions of uniq
    {
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

        for(int i = 0; i < vertUsed.size(); i++)
        {
            if(vertUsed[i] == true) nVertInds.push_back(i);
        }
    }

    std::vector newIndices_by_oldIndices(verts.size(), -1); //Map the old indices to the new indices
    for(int i = 0; i < nVertInds.size(); i++)
    {
        newIndices_by_oldIndices[nVertInds[i]] = i;
    }
    auto nverts = subset(verts, nVertInds); //New vertices (can be mapped)
    std::vector<std::vector<int>> nsegs2;
    {
        for(const auto& seg : nsegs)
        {
            nsegs2.push_back({});
            for(const auto& pt : seg)
            {
                nsegs2[nsegs2.size() - 1].push_back(newIndices_by_oldIndices[pt]);
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
