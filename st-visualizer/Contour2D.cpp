#include <chrono>
#include "Contour2D.h"

#define BEGIN std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now(); int count = 0;

#define TOCK std::cout << count++ << ", Time difference = " << static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - begin).count())/1000 << " [milliseconds]" << std::endl; begin = std::chrono::steady_clock::now();

int orientation(Eigen::Vector2f a, Eigen::Vector2f b)
{
    Eigen::Vector3f intermediateA;
    intermediateA(0) = a(0);
    intermediateA(1) = a(1);
    intermediateA(2) = 0;

    Eigen::Vector3f intermediateB;
    intermediateB(0) = b(0);
    intermediateB(1) = b(1);
    intermediateB(2) = 0;
    const auto result = intermediateA.cross(intermediateB).eval()(2);
    return result == 0.0f ? 0 : (result < 0.0f ? -1 : 1);
}

int getMaxPos(const std::vector<float>& vals)
{
    int maxIndex = 0;
    for (int i = 0; i < vals.size(); i++)
    {
        if (vals[maxIndex] < vals[i])
        {
            maxIndex = i;
        }
    }

    return maxIndex;
}

contourTriMultiDCStruct contourTriMultiDC(Eigen::Matrix2Xf pointIndexToPoint, std::vector<std::vector<int>> triangleIndexToCornerIndices, std::vector<std::vector<float>> pointIndexToMaterialValues)
{

    
    ////////////
    //Step 1: Set up a structure to define geometry
    ////////////
    BEGIN
    const auto numberOfTriangles = triangleIndexToCornerIndices.size();

    //This is a list of the combinations of edges on a triangle by corner
    const std::vector<std::pair<int, int>> triangle_edges = { {0, 1}, {1, 2}, {2, 0} };

    //Primary Material index at each point
    auto primaryMaterialIndexByPointIndex = pointIndexToMaterialValues << std::function(getMaxPos);

    const size_t number_of_points = pointIndexToPoint.cols();
    std::vector endpointIndicesToEdgeIndex(number_of_points, std::vector(number_of_points, -1));

    //Might be good to eventually make this a pair

    std::vector<std::vector<int>> edgeIndexToEndpointIndices;
    //Stores which face index an edge belongs to
    std::vector<std::vector<int>> edgeIndexToFaceIndices;
    //TODO: This is the longest process
    //Make connections between edges/faces, edges/endpoints, and endpoints/edges
    for (int triangleSide = 0; triangleSide < 3; triangleSide++)
    {
        for (size_t faceIndex = 0; faceIndex < numberOfTriangles; faceIndex++)
        {
            auto endpointIndices = std::pair(
                triangleIndexToCornerIndices[faceIndex][triangle_edges[triangleSide].first],
                triangleIndexToCornerIndices[faceIndex][triangle_edges[triangleSide].second]
            );


            if (endpointIndicesToEdgeIndex[endpointIndices.first][endpointIndices.second]==-1)//If the edge doesn't already exist
            {
                int num_of_edges = edgeIndexToEndpointIndices.size();

                const auto& cornerPair = triangle_edges[triangleSide];
                edgeIndexToEndpointIndices.push_back({
                    triangleIndexToCornerIndices[faceIndex][cornerPair.first],
                    triangleIndexToCornerIndices[faceIndex][cornerPair.second]
                });
                edgeIndexToFaceIndices.push_back({static_cast<int>(faceIndex)});
                endpointIndicesToEdgeIndex[endpointIndices.first][endpointIndices.second] = num_of_edges;
                endpointIndicesToEdgeIndex[endpointIndices.second][endpointIndices.first] = num_of_edges;
                num_of_edges++;
            }
            else
            {
                int existingEdgeIndex = endpointIndicesToEdgeIndex[endpointIndices.first][endpointIndices.second];
                edgeIndexToFaceIndices[existingEdgeIndex].push_back(faceIndex); //Connect it to it's other face
            }
        }
    }


    /*create interpolation points if they exist, one per edge with material change*/

    std::vector<std::pair<Eigen::Vector2f, bool>> edgeIndexToMidPoints(edgeIndexToFaceIndices.size(), { {0,0},false }); //Second value is whether it has been set
    for (int edgeIndex = 0; edgeIndex < edgeIndexToEndpointIndices.size(); edgeIndex++)
    {
        const auto& endpointIndices = edgeIndexToEndpointIndices[edgeIndex];

        // If the two materials at the adjacent points are different
        if (primaryMaterialIndexByPointIndex[endpointIndices[0]] != primaryMaterialIndexByPointIndex[endpointIndices[1]])
        {
            const auto& endpt0Index = endpointIndices[0];
            const auto& endpt1Index = endpointIndices[1];

            const auto& endpt0PrimaryValueIndex = primaryMaterialIndexByPointIndex[endpt0Index];
            const auto& endpt1PrimaryValueIndex = primaryMaterialIndexByPointIndex[endpt1Index];

            const auto& primaryValues0 = pointIndexToMaterialValues[endpt0Index];
            const auto& primaryValues1 = pointIndexToMaterialValues[endpt1Index];

            edgeIndexToMidPoints[edgeIndex] = { interpEdge2Mat(
                pointIndexToPoint.col(endpointIndices[0]),
                pointIndexToPoint.col(endpointIndices[1]),
                { primaryValues0[endpt0PrimaryValueIndex], primaryValues0[endpt1PrimaryValueIndex] },
                { primaryValues1[endpt0PrimaryValueIndex], primaryValues1[endpt1PrimaryValueIndex] }
            ), true};
        }
    }
    

    std::vector edgeIndexToFacePointIndex(edgeIndexToEndpointIndices.size(), -1);
    /*create vertices in faces, one per triangle with material change*/
    //TODO: Increase speed
    //1) Make slice tests
    //2) Make tests on real data
    //3) Time Benchmark for speed
    std::vector<Eigen::Vector2f> facePointByIndex;
    facePointByIndex.reserve(numberOfTriangles*2);
    std::vector triangleIndexToFacePointIndex(numberOfTriangles, -1);
    for (size_t triangleIndex = 0; triangleIndex < triangleIndexToCornerIndices.size(); triangleIndex++) {
        auto cornerIndices = triangleIndexToCornerIndices[triangleIndex];
        //If there are material changes in the triangle
        if (primaryMaterialIndexByPointIndex[cornerIndices[0]] != primaryMaterialIndexByPointIndex[cornerIndices[1]] || primaryMaterialIndexByPointIndex[cornerIndices[1]] != primaryMaterialIndexByPointIndex[cornerIndices[2]])
        {
            //Generate the center point of the existing midpoint
            std::vector<Eigen::Vector2f> triangleMidpoints;
            for (auto edge : triangle_edges)
            {
                auto endpointIndices = endpointIndicesToEdgeIndex[cornerIndices[edge.first]][ cornerIndices[edge.second] ];
                if (edgeIndexToMidPoints[endpointIndices].second) triangleMidpoints.push_back(edgeIndexToMidPoints[endpointIndices].first);
            }

            auto centerPoint = getMassPoint(triangleMidpoints);
            facePointByIndex.push_back(centerPoint);

            //Create a face vertex at that generated center point
            triangleIndexToFacePointIndex[triangleIndex] = static_cast<int>(facePointByIndex.size()) - 1;
        }
    }
    
    /*create center segments*/
    std::vector<std::pair<int, int>> centerSegmentIndexToEndpointIndices;
    centerSegmentIndexToEndpointIndices.reserve(facePointByIndex.size() * 2);
    std::vector<std::pair<int, int>> centerSegmentToEndpointPrimaryMaterialIndices;
    centerSegmentToEndpointPrimaryMaterialIndices.reserve(facePointByIndex.size() * 2);

    for (int edge_index = 0; edge_index < edgeIndexToEndpointIndices.size(); edge_index++)
    {
        //If there is a change in material between the edges of the segments
        if (primaryMaterialIndexByPointIndex[edgeIndexToEndpointIndices[edge_index][0]] != primaryMaterialIndexByPointIndex[edgeIndexToEndpointIndices[edge_index][1]])
        {
            //The index is going to be the size of the list of segments before the push
            const int newSegmentIndex = centerSegmentIndexToEndpointIndices.size();

            const auto& endpointIndices = edgeIndexToEndpointIndices[edge_index];
            centerSegmentToEndpointPrimaryMaterialIndices.push_back({
                primaryMaterialIndexByPointIndex[endpointIndices[0]],
                primaryMaterialIndexByPointIndex[endpointIndices[1]]
            });

            std::pair<int, int> newSegmentEndpointIndices;
            if (edgeIndexToFaceIndices[edge_index].size() == 1)
            {
                //Boundary edge, connect edge point and triangle point
                facePointByIndex.push_back(edgeIndexToMidPoints[edge_index].first);
                const int facePointIndex = facePointByIndex.size() - 1;
                edgeIndexToFacePointIndex[edge_index] = facePointIndex;
                newSegmentEndpointIndices = { triangleIndexToFacePointIndex[edgeIndexToFaceIndices[edge_index][0]], facePointIndex };
            }
            else
            {
                newSegmentEndpointIndices = { triangleIndexToFacePointIndex[edgeIndexToFaceIndices[edge_index][0]], triangleIndexToFacePointIndex[edgeIndexToFaceIndices[edge_index][1]] };
            }

            //Ensure consistent orientation among endpoints
            if (orientation(pointIndexToPoint.col(edgeIndexToEndpointIndices[edge_index][0]) - pointIndexToPoint.col(edgeIndexToEndpointIndices[edge_index][1]), facePointByIndex[newSegmentEndpointIndices.first] - pointIndexToPoint.col(edgeIndexToEndpointIndices[edge_index][1])) < 0)
            {
                newSegmentEndpointIndices = { newSegmentEndpointIndices.second, newSegmentEndpointIndices.first };
            }

            centerSegmentIndexToEndpointIndices.push_back(std::move(newSegmentEndpointIndices));
        }
    }


    /*Create Fill Triangles -> Solid fill triangles for displaying areas of material type, not the contour*/
    std::vector<Eigen::Vector2f> resultingPointsByIndex;
    resultingPointsByIndex.reserve(facePointByIndex.size() + pointIndexToPoint.cols()); //We know how big this will be
    for (const auto& centerPoint : facePointByIndex)
    {
        resultingPointsByIndex.push_back(centerPoint);
    }
    //Join both sets of points into all the existing points
    for (const auto& pt : pointIndexToPoint.colwise())
    {
        resultingPointsByIndex.emplace_back(pt);
    }
    std::vector<std::vector<int>> resultingTriangleIndexToResultingCornerIndices;
    resultingTriangleIndexToResultingCornerIndices.reserve(numberOfTriangles * 6);
    std::vector<int> fillMats;
    fillMats.reserve(numberOfTriangles * 6);

    /*first type of triangles : dual to mesh edges with a material change*/
    for (int currentEdgeIndex = 0; currentEdgeIndex < edgeIndexToEndpointIndices.size(); currentEdgeIndex++)
    {
        //If the materials on either end of an edge don't match, there will be a triangle
        if (primaryMaterialIndexByPointIndex[edgeIndexToEndpointIndices[currentEdgeIndex][0]] != primaryMaterialIndexByPointIndex[edgeIndexToEndpointIndices[currentEdgeIndex][1]])
        {
            std::pair<int, int> segmentEndpoints;
            if (edgeIndexToFaceIndices[currentEdgeIndex].size() == 1) //if boundary edge edge
            {
                if (edgeIndexToFacePointIndex[currentEdgeIndex] == -1) throw "Logic error, midpoint index not found in face points";
                /*boundary edge : connect edge point and center point*/
                segmentEndpoints = { triangleIndexToFacePointIndex[edgeIndexToFaceIndices[currentEdgeIndex][0]], edgeIndexToFacePointIndex[currentEdgeIndex] };
            }
            else
            {
                /*interior edge : connect two center points*/
                //We know both of these will exist b/c there is always a center point if the triangle has any material changes
                segmentEndpoints = {
                    triangleIndexToFacePointIndex[edgeIndexToFaceIndices[currentEdgeIndex][0]],
                    triangleIndexToFacePointIndex[edgeIndexToFaceIndices[currentEdgeIndex][1]]
                };
            }

            //Insert two triangles&materials using the two endpoints of the currentEdge as the third corner

            //Insert the "top" triangle
            resultingTriangleIndexToResultingCornerIndices.push_back(std::vector({
                segmentEndpoints.first,
                segmentEndpoints.second,
                static_cast<int>(facePointByIndex.size()) + edgeIndexToEndpointIndices[currentEdgeIndex][0] //B/c the edge indices start after the face point indices
                }));
            fillMats.push_back(primaryMaterialIndexByPointIndex[edgeIndexToEndpointIndices[currentEdgeIndex][0]]);

            //Insert the "bottom" triangle
            resultingTriangleIndexToResultingCornerIndices.push_back(std::vector({
                segmentEndpoints.first,
                segmentEndpoints.second,
                static_cast<int>(facePointByIndex.size()) + edgeIndexToEndpointIndices[currentEdgeIndex][1]
                }));
            fillMats.push_back(primaryMaterialIndexByPointIndex[edgeIndexToEndpointIndices[currentEdgeIndex][1]]);
        }
    }
    
    /* second type of triangles: original mesh triangle, if there is no material change,
     or a third of the triangle, if there is some edge with no material change */
    auto temp = std::function([facePointByIndex](int item) { return item + static_cast<int>(facePointByIndex.size()); });
    for (int currentTriangleIndex = 0; currentTriangleIndex < triangleIndexToCornerIndices.size(); currentTriangleIndex++)
    {
        //If There is no change at all
        if (primaryMaterialIndexByPointIndex[triangleIndexToCornerIndices[currentTriangleIndex][1]] == primaryMaterialIndexByPointIndex[triangleIndexToCornerIndices[currentTriangleIndex][2]] && primaryMaterialIndexByPointIndex[triangleIndexToCornerIndices[currentTriangleIndex][2]] == primaryMaterialIndexByPointIndex[triangleIndexToCornerIndices[currentTriangleIndex][0]])
        {
            resultingTriangleIndexToResultingCornerIndices.push_back(triangleIndexToCornerIndices[currentTriangleIndex] << temp);
            fillMats.push_back(primaryMaterialIndexByPointIndex[triangleIndexToCornerIndices[currentTriangleIndex][0]]);
        }
        else
        {
            for (int j = 0; j < 3; j++)
            {
                //If one of the edges has no change

                const auto& triangle_side = triangle_edges[j];
                const auto& cornerIndices = triangleIndexToCornerIndices[currentTriangleIndex];
                if (primaryMaterialIndexByPointIndex[cornerIndices[triangle_side.first]] ==
                    primaryMaterialIndexByPointIndex[cornerIndices[triangle_side.second]])
                {
                    resultingTriangleIndexToResultingCornerIndices.push_back( {
                        cornerIndices[triangle_side.first] + static_cast<int>(facePointByIndex.size()),
                        cornerIndices[triangle_side.second] + static_cast<int>(facePointByIndex.size()),
                        triangleIndexToFacePointIndex[currentTriangleIndex] });

                    fillMats.push_back(primaryMaterialIndexByPointIndex[triangleIndexToCornerIndices[currentTriangleIndex][triangle_side.second]]);
                }
            }
        }
    }
    TOCK

    //Test on three points, four points, and a ring around a single point (triangulated)

    return {
        std::move(facePointByIndex),
        std::move(centerSegmentIndexToEndpointIndices),
        std::move(centerSegmentToEndpointPrimaryMaterialIndices),
        std::move(resultingPointsByIndex),
        std::move(resultingTriangleIndexToResultingCornerIndices),
        std::move(fillMats) };
}
