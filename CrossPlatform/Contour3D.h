#pragma once
#include "Contour2D.h"
#include "UtilityFunctions.h"
#include "tetgen1.6.0/tetgen.h"
//#include <string.h>

#define LOADING_SIZE 10
#define DEBUG true
#ifndef __nullptr
#define __nullptr nullptr
#endif

//1) Generate vertices, don't append, create flat array w/ fixed size
//2) Remove 2d/3d vector to hashmaps (DONE)
//3) Store indices of the other two vertices not on the edge (DONE)
//4) Figure out how to reduce number of data structures

template <typename T>
class rigidVector
{
    T* data = __nullptr;
    size_t max = 0;
    size_t s = 0;
public:
    rigidVector()
    {
    }

    void reserve(size_t newMax)
    {
        max = newMax;
        delete[] data;
        data = new T[newMax];
    }

    T* start() { return data + 0; }
    T* end() { return data + s; }

    T& operator[](size_t index)
    {
        return data[index];
    }

    void push_back(const T& i)
    {
        data[s++] = i;
    }

    void push_back(T&& i)
    {
        data[s++] = std::move(i);
        if(s > max) throw "Out of Range";
    }
};

inline int orientation(const Eigen::Vector3f& u, const Eigen::Vector3f& v, const Eigen::Vector3f& w)
{
    auto val = u.cross(v).dot(w);
    if(val < 0) return -1;
    if(val > 0) return 1;
    return 0;
}

//Slower, but doesn't copy construct. Use when vectors are small.
inline std::vector<int> complementByReference(const std::vector<int>& source, const std::vector<int>& target)
{
    std::vector<int> accumulator;
    accumulator.reserve(source.size());
    for(const auto& item : source)
    {
        bool wasFound = false;
        for(const auto& t : target)
        {
            if(t == item) wasFound = true;
        }
        if(!wasFound) accumulator.push_back(item);
    }
    std::sort(accumulator.begin(), accumulator.end());
    return accumulator;
}


//O(nlog(n)+mlog(m) operation, but every usage n<=m<=4, so probably not a problem
inline std::vector<int> complement(std::vector<int> source, std::vector<int> target)
{
    std::sort(source.begin(), source.end());
    std::sort(target.begin(), target.end());

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

inline std::pair<std::vector<int>, std::vector<int>> orderTets(const std::pair<int, int>& edge,
                                                               const std::vector<std::vector<int>>& tets)
{
    std::pair<std::vector<int>, std::vector<int>> ret;
    auto& orderedTets = ret.first;
    orderedTets.reserve(tets.size());

    //Which 2 other corners does each tet have?
    std::vector<std::vector<int>> cornersByTet; //This should have two non-edge vertices per tet
    {
        cornersByTet.reserve(tets.size());
        std::vector edgeEndpoints = {edge.first, edge.second};
        for(const auto& tet : tets)
        {
            cornersByTet.push_back(complementByReference(tet, edgeEndpoints));
        }
    }


    //Stores which tet(s) each corner belongs to
    //Which tet, which index in tet
    std::unordered_map<int, std::vector<std::pair<int, int>>> tets_by_corner;
    {
        //For Each tet
        //O(2n) b/c each tet has exactly two stored corners
        size_t corner_index = 0;
        for(int tetIndex = 0; tetIndex < cornersByTet.size(); tetIndex++) //O(n)
        {
            //For each of the two corners in that tet
            for(corner_index = 0; corner_index < cornersByTet[tetIndex].size(); corner_index++) //O(2)
            {
                //Push back the corners that it touches
                const auto& corner = cornersByTet[tetIndex][corner_index];
                tets_by_corner[corner].reserve(10);
                tets_by_corner[corner].emplace_back(tetIndex, corner_index);
            }
        }
    }

    //Find the two endpoints (if they exist)
    auto& endpoints = ret.second;
    {
        //For each entry in tets_by_corner
        for(const auto& [corner, owningTets] : tets_by_corner)
        {
            if(owningTets.size() == 1) //If the corner only belongs to one tet
            {
                endpoints.push_back(corner); //Then save it as an endpoint

                if(endpoints.size() == 2) break; //We know there will be 0 or 2
            }
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
        const auto& temp = tets_by_corner[endpoints[0]][0]; //The tet associated with the first unique corner
        orderedTets.push_back(temp.first);
        nextCorner = cornersByTet[temp.first][1 - temp.second]; //Start with the other corner in that tet
        endPoint = endpoints[1]; //End with the other unique corner
    }

    //Run through all the connected tets to order them correctly
    //Stop when we get to the endpoint (Which will be on the first tet we started at or on the other unconnected tet)
    while(nextCorner != endPoint) //O(n)
    {
        const auto& tet_entry = tets_by_corner[nextCorner];

        //If we have already looked at the first tet in the entry, then we know it is the second and vice versa
        //First is tet index, second is the corner's index in that tet (0 or 1 b/c we removed the two we are rotating around)
        auto [tet_index, corner_index] = tet_entry[0].first == orderedTets[orderedTets.size() - 1]
                                         ? tet_entry[1]
                                         : tet_entry[0];

        orderedTets.push_back(tet_index); //Push back the tet index
        nextCorner = cornersByTet[tet_index][static_cast<size_t>(1) - corner_index];
        //Save the other corner from that tet
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
        table.reserve(size);
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
    //TODO: This is slow
    int& at(const int& a, const int& b)
    {
        if(a > b)
        {
            if(!table[b].contains(a))
            {
                table[b][a] = -1;
            }
            return table[b][a];
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
    log("Contouring.");
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
    edge_index_by_endpoint_indices.table.reserve(points_by_index.size());
    std::vector<std::pair<int, int>> edges_by_index;
    std::vector<std::vector<int>> tets_by_edge_index;
    {
        edges_by_index.reserve(number_of_tets * 6);
        tets_by_edge_index.reserve(number_of_tets * 6);
        const int display_fraction = number_of_tets % LOADING_SIZE == 0
                                     ? number_of_tets / LOADING_SIZE
                                     : (number_of_tets / LOADING_SIZE) + 1;

        //For each tet
        //O(n)
        log("Mapping Tets to Edges");
        for(size_t tet_index = 0; tet_index < number_of_tets; tet_index++)
        {
            const auto& tet = tets_by_index[tet_index];

            //For each edge in that tet
            //O(6)
            for(const auto& corner_combination : corner_combinations)
            {
                //Get the edge's endpoints
                const auto& firstEndpoint = tet[corner_combination.first];
                const auto& secondEndpoint = tet[corner_combination.second];

                //See if we have looked at this edge before
                auto& edge_index = edge_index_by_endpoint_indices.at(firstEndpoint, secondEndpoint);

                if(edge_index == -1) //If the edge does not exist
                {
                    //Add it to the various indexes we are building
                    edge_index = static_cast<int>(edges_by_index.size());

                    edges_by_index.emplace_back(firstEndpoint, secondEndpoint);

                    tets_by_edge_index.emplace_back();
                    tets_by_edge_index[edge_index].reserve(10);
                }

                //Track the tet part no matter what
                tets_by_edge_index[edge_index].push_back(tet_index);
            }
            if (tet_index % display_fraction == 0)
                log("  ", static_cast<float>(100 * tet_index / display_fraction) / LOADING_SIZE, "%");

        }
    }

    //create interpolation points_by_index, one per edge with material change
    std::unordered_map<int, Eigen::Vector3f> edgePoint_by_edge_index;
    {
        edgePoint_by_edge_index.reserve(edges_by_index.size());
        //O(n)
        for(int i = 0; i < edges_by_index.size(); i++)
        {
            auto& edge = edges_by_index[i];
            const auto& p_material_index = primary_material_by_point_index[edge.first];
            const auto& q_material_index = primary_material_by_point_index[edge.second];

            //If there is a change between the p and q materials
            if(p_material_index != q_material_index)
            {
                const auto& p = points_by_index[edge.first];
                const auto& q = points_by_index[edge.second];
                const auto& pvals = vals_by_point_index[edge.first];
                const auto& qvals = vals_by_point_index[edge.second];

                //Calculate the difference between them
                edgePoint_by_edge_index[i] = interpEdge2Mat<3>(
                    p, q,
                    {pvals[p_material_index], pvals[q_material_index]},
                    {qvals[p_material_index], qvals[q_material_index]}
                );
            }
        }
    }

    //create vertices, one per tet with material change
    std::vector<Eigen::Vector3f> vertex_by_index;
    std::vector<int> vertex_index_by_tet_index;
    {
        std::vector<Eigen::Vector3f> temp;
        vertex_by_index.reserve(number_of_tets * 6);
        vertex_index_by_tet_index.reserve(number_of_tets);
        for(const auto& tet : tets_by_index)
        {
            //If there is no material change within the tet
            if(primary_material_by_point_index[tet[0]] == primary_material_by_point_index[tet[1]]
                && primary_material_by_point_index[tet[0]] == primary_material_by_point_index[tet[2]]
                && primary_material_by_point_index[tet[0]] == primary_material_by_point_index[tet[3]])
            {
                vertex_index_by_tet_index.push_back(-1); //There is no central vertex
            }
            else
            {
                temp.reserve(corner_combinations.size());
                for(const auto& corner_combination : corner_combinations)
                {
                    const auto& pointIndex = edge_index_by_endpoint_indices.at(
                        tet[corner_combination.first], tet[corner_combination.second]);

                    if(edgePoint_by_edge_index.contains(pointIndex)) temp.
                        push_back(edgePoint_by_edge_index[pointIndex]);
                }
                vertex_by_index.push_back(getMassPoint<3>(temp));
                vertex_index_by_tet_index.push_back(vertex_by_index.size() - 1);
                temp.resize(0);
            }
        }
    }

    //Create Segments

    Hash3d bdFaceHash(edges_by_index.size());
    std::vector<std::vector<int>> segments_by_index;
    std::vector<std::pair<int, int>> segmentMaterials_by_index;
    {
        log("Creating Contour Segements.");
        segments_by_index.reserve(edges_by_index.size());
        segmentMaterials_by_index.reserve(edges_by_index.size());

        const std::vector<std::pair<int, int>> triangleEdges = {{0, 1}, {1, 2}, {2, 0}};

        const int print_constant = edges_by_index.size() % LOADING_SIZE == 0
                                   ? edges_by_index.size() / LOADING_SIZE
                                   : (edges_by_index.size() / LOADING_SIZE) + 1;

        //O(n), total O(n^2)
        //This function has the largest slowdown
        for(int edgeIndex = 0; edgeIndex < edges_by_index.size(); edgeIndex++)
        {
            const auto& edge = edges_by_index[edgeIndex];

            if(primary_material_by_point_index[edge.first] != primary_material_by_point_index[edge.second])
            {
                segmentMaterials_by_index.emplace_back(primary_material_by_point_index[edge.first],
                                                       primary_material_by_point_index[edge.second]);

                const auto& tets_around_edge_by_index = tets_by_edge_index[edgeIndex];
                const auto& tets_surrounding_edge_set = subset(tets_by_index, tets_around_edge_by_index);

                //O(n)
                const auto [ordered_tets_in_set, endpoints] = orderTets(edge, tets_surrounding_edge_set);

                //Each tet surrounding the edge
                const auto& tets_touching_edge_set = subset(tets_around_edge_by_index, ordered_tets_in_set);

                //Stores the vertices in order that make up the new segment
                std::vector<int> new_segment_vertices_set_ordered = subset(
                    vertex_index_by_tet_index, tets_touching_edge_set);

                if(!endpoints.empty())
                {
                    //if there are boundary faces
                    std::vector endpoint_vertices = {-1, -1};
                    //O(2)
                    for(auto j = 0; j < endpoint_vertices.size(); j++)
                    {
                        std::vector tri = {edge.first, edge.second, endpoints[j]};
                        std::sort(tri.begin(), tri.end());

                        auto& hashValue = bdFaceHash.at(tri[0], tri[1], tri[2]);
                        endpoint_vertices[j] = hashValue;

                        //If it doesn't exist
                        if(hashValue == -1)
                        {
                            //Add Face Vertex
                            std::vector<Eigen::Vector3f> massedPoints;
                            massedPoints.reserve(triangleEdges.size());
                            //O(3)
                            for(const auto& triangleEdgeEndpoints : triangleEdges)
                            {
                                auto edge_index = edge_index_by_endpoint_indices.at(
                                    tri[triangleEdgeEndpoints.first], tri[triangleEdgeEndpoints.second]);
                                if(edgePoint_by_edge_index.contains(edge_index))
                                    massedPoints.push_back(edgePoint_by_edge_index[edge_index]);
                            }
                            vertex_by_index.push_back(getMassPoint<3>(massedPoints));
                            endpoint_vertices[j] = vertex_by_index.size() - 1;
                            hashValue = endpoint_vertices[j];
                        }
                    }

                    new_segment_vertices_set_ordered = concat(
                        concat({endpoint_vertices[0]}, new_segment_vertices_set_ordered), {endpoint_vertices[1]});
                }


                //orient the polygon
                int p1, p2;
                if(ordered_tets_in_set.size() == 1)
                {
                    p1 = endpoints[0];
                    p2 = endpoints[1];
                }
                else
                {
                    const auto& random_tet = tets_by_index[tets_around_edge_by_index[ordered_tets_in_set[0]]];
                    p1 = complementByReference(random_tet,
                                               tets_by_index[tets_around_edge_by_index[ordered_tets_in_set[1]]])[0];
                    p2 = complementByReference(random_tet,
                                               {edge.first, edge.second, p1})[0];
                }

                if(orientation(points_by_index[edge.first] - points_by_index[edge.second],
                               points_by_index[p1] - points_by_index[edge.first],
                               points_by_index[p2] - points_by_index[p1]) < 0)
                {
                    std::reverse(new_segment_vertices_set_ordered.begin(), new_segment_vertices_set_ordered.end());
                }

                segments_by_index.push_back(new_segment_vertices_set_ordered);
            }

            if (edgeIndex % print_constant == 0)
                log("  ", static_cast<float>(100 * edgeIndex / print_constant) / LOADING_SIZE, "%");
            

        }
    }
    return {vertex_by_index, segments_by_index, segmentMaterials_by_index};
}


inline void tetralizeMatrix(const Eigen::Matrix3Xf& pts, tetgenio& out)
{
    log("Tetrahedralizing Data.");
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
    tetrahedralize(strdup(
                       "z" //Start arrays at zero
                       // "V" //Verbose for debugging
                       "Q" //Quiet for production
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
    std::vector<int> first_index_match;
    std::vector<int> second_index_match;
    {
        for(int i = 0; i < segs.size(); i++)
        {
            if(segmats[i].first == mat) first_index_match.push_back(i);
            if(segmats[i].second == mat) second_index_match.push_back(i);
        }
    }

    auto reversed_second_matches = subset(segs, second_index_match);
    for(auto& seg : reversed_second_matches)
    {
        std::reverse(seg.begin(), seg.end());
    }
    auto matching_material_segments = concat(subset(segs, first_index_match), reversed_second_matches);
    //All these segements start with the target material

    std::vector<int> oldIndices_by_newIndex; //Indices of the vertices we care about
    {
        std::vector vertices_used_by_index(verts.size(), false);
        {
            //O(n)
            for(const auto& seg : matching_material_segments)
            {
                //O(2)
                for(const auto& ind : seg)
                {
                    vertices_used_by_index[ind] = true;
                }
            }
        }
        //O(n)
        for(int i = 0; i < vertices_used_by_index.size(); i++)
        {
            if(vertices_used_by_index[i] == true) oldIndices_by_newIndex.push_back(i);
        }
    }

    std::vector newIndices_by_oldIndex(verts.size(), -1); //Map the old indices to the new indices
    for(int i = 0; i < oldIndices_by_newIndex.size(); i++)
    {
        newIndices_by_oldIndex[oldIndices_by_newIndex[i]] = i;
    }
    auto new_vertices = subset(verts, oldIndices_by_newIndex);

    //New vertices (can be mapped)
    std::vector<std::vector<int>> revised_new_segments;
    {
        for(const auto& seg : matching_material_segments)
        {
            revised_new_segments.emplace_back();
            auto& newItem = revised_new_segments[revised_new_segments.size() - 1];
            newItem.reserve(seg.size());

            for(const auto& pt : seg)
            {
                newItem.push_back(newIndices_by_oldIndex[pt]);
            }
        }
    }

    //shrink
    std::vector<Eigen::Vector3f> vertex_normals(new_vertices.size(), {0, 0, 0});
    for(const auto& seg : revised_new_segments)
    {
        std::vector<Eigen::Vector3f> points = subset(new_vertices, seg);
        auto nm = getFaceNorm(points);
        for(auto index : seg)
        {
            vertex_normals[index] += nm;
        }
    }

    for(size_t i = 0; i < vertex_normals.size(); i++)
    {
        const auto faceVector = vertex_normals[i];
        const auto normalized = faceVector.normalized();
        new_vertices[i] += shrink * normalized;
    }

    return {new_vertices, revised_new_segments};
}

inline std::vector<std::pair<std::vector<Eigen::Vector3f>, std::vector<std::vector<int>>>> getContourAllMats3D(
    const std::vector<Eigen::Vector3f>& verts,
    const std::vector<std::vector<int>>& segs,
    const std::vector<std::pair<int, int>>& segmats,
    const int& number_of_materials,
    const float& shrink)
{
    std::vector<std::pair<std::vector<Eigen::Vector3f>, std::vector<std::vector<int>>>> a;
    for(int i = 0; i < number_of_materials; i++)
    {
        a.push_back(getContourByMat3D(verts, segs, segmats, i, shrink));
    }
    return a;
}
