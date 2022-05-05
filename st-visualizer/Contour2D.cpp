
#include "Contour2D.h"
#include <iostream>
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
            maxIndex = 1;
        }
    }

    return maxIndex;
}

contourTriMultiDCStruct contourTriMultiDC(Eigen::Matrix2Xf pointIndexToPoint, std::vector<std::vector<int>> triangleIndexToCornerIndices, std::vector<std::vector<float>> pointIndexToMaterialValues)
{
    ////////////
    //Step 1: Set up a structure to define geometry
    ////////////
    const auto numberOfMaterials = pointIndexToMaterialValues[0].size();
    const auto numberOfPoints = pointIndexToPoint.cols();
    const auto numberOfTriangles = triangleIndexToCornerIndices.size();

    //This is a list of the combinations of edges on a triangle by corner
    const std::vector<std::vector<int>> triangle_edges = { {0, 1}, {1, 2}, {2, 0} };

    //Primary Material index at each point
    auto primaryMaterialIndexByPointIndex = pointIndexToMaterialValues << std::function(getMaxPos);

    std::map<std::pair<int, int>, int> endpointIndicesToEdgeIndex;

    //Might be good to eventually make this a pair

    auto edgeIndexToEndpointIndices = table(numberOfTriangles * 3, std::function([](size_t) { return std::vector<int>(); }));
    //Stores which face index an edge belongs to
    auto edgeIndexToFaceIndices = table(numberOfTriangles * 3, std::function([](size_t) { return std::vector<int>(); }));
    auto edgeIndex = 0;

    //Make connections between edges/faces, edges/endpoints, and endpoints/edges
    for (int triangleSide = 0; triangleSide < 3; triangleSide++)
    {
        for (int faceIndex = 0; faceIndex < numberOfTriangles; faceIndex++)
        {
            auto endpointIndices = std::pair(
                triangleIndexToCornerIndices[faceIndex][triangle_edges[triangleSide][0]],
                triangleIndexToCornerIndices[faceIndex][triangle_edges[triangleSide][1]]
            );


            if (endpointIndicesToEdgeIndex.contains(endpointIndices))//If the edge doesn't already exist
            {
                //here ct is a unique index for an edge which is stored in edges
                edgeIndexToEndpointIndices[edgeIndex] = triangle_edges[triangleSide] << std::function([triangleIndexToCornerIndices, faceIndex](const int& index) { return triangleIndexToCornerIndices[faceIndex][index]; });
                edgeIndexToFaceIndices[edgeIndex] = { faceIndex };
                endpointIndicesToEdgeIndex[{endpointIndices.first, endpointIndices.second}] = edgeIndex;
                endpointIndicesToEdgeIndex[{endpointIndices.second, endpointIndices.first}] = edgeIndex;
                edgeIndex++;
            }
            else
            {
                int existingEdgeIndex = endpointIndicesToEdgeIndex[endpointIndices];
                edgeIndexToFaceIndices[existingEdgeIndex].push_back(faceIndex); //Connect it to it's other face
            }
        }
    }

    //There should be ct number of elements in the vector now
    //This could be made more efficient by avoiding constructing a new vector
    edgeIndexToEndpointIndices = std::vector(edgeIndexToEndpointIndices.begin(), edgeIndexToEndpointIndices.begin() + edgeIndex);
    edgeIndexToFaceIndices = std::vector(edgeIndexToFaceIndices.begin(), edgeIndexToFaceIndices.begin() + edgeIndex);


    /*create interpolation points if they exist, one per edge with material change*/

    std::map<int, Eigen::Vector2f> edgeIndexToMidPoints;
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

            edgeIndexToMidPoints[edgeIndex] = interpEdge2Mat(
                pointIndexToPoint.col(endpointIndices[0]),
                pointIndexToPoint.col(endpointIndices[1]),
                { primaryValues0[endpt0PrimaryValueIndex], primaryValues0[endpt1PrimaryValueIndex] },
                { primaryValues1[endpt0PrimaryValueIndex], primaryValues1[endpt1PrimaryValueIndex] }
            );
        }
    }
    std::vector midpointIndexToEdgeIndex(edgeIndexToEndpointIndices.size(), 0);

    /*create vertices in faces, one per triangle with material change*/
    std::vector<Eigen::Vector2f> centerPointByIndex;
    int centerPointIndex = 0;
    std::map<int, int> triangleIndexToCenterPointIndex;
    for (int triangleIndex = 0; triangleIndex < triangleIndexToCornerIndices.size(); triangleIndex++) {
        auto cornerIndices = triangleIndexToCornerIndices[triangleIndex];
        //If there are material changes in the triangle
        if (primaryMaterialIndexByPointIndex[cornerIndices[0]] != primaryMaterialIndexByPointIndex[cornerIndices[1]] || primaryMaterialIndexByPointIndex[cornerIndices[1]] != primaryMaterialIndexByPointIndex[cornerIndices[2]])
        {
            //Generate the center point of the existing midpoint
            std::vector<Eigen::Vector2f> triangleMidpoints;
            for (auto edge : triangle_edges)
            {
                auto endpointIndices = endpointIndicesToEdgeIndex[{cornerIndices[edge[0]], cornerIndices[edge[1]]}];
                if (edgeIndexToMidPoints.contains(endpointIndices)) triangleMidpoints.push_back(edgeIndexToMidPoints[endpointIndices]);
            }

            auto centerPoint = getMassPoint(triangleMidpoints);
            centerPointByIndex.push_back(centerPoint);

            //Create a face vertex at that generated center point
            triangleIndexToCenterPointIndex[triangleIndex] = static_cast<int>(centerPointByIndex.size()) - 1;
        }
    }

    /*create center segments*/
    std::vector<std::pair<int, int>> centerSegmentIndexToEndpointIndices;
    std::vector<std::pair<int, int>> centerSegmentToEndpointPrimaryMaterialIndices;
    for (int edge_index = 0; edge_index < edgeIndexToEndpointIndices.size(); edge_index++)
    {
        //If there is a change in material between the edges of the segments
        if (primaryMaterialIndexByPointIndex[edgeIndexToEndpointIndices[edge_index][0]] == primaryMaterialIndexByPointIndex[edgeIndexToEndpointIndices[edge_index][1]])
        {
            //The index is going to be the size of the list of segments before the push
            const int newSegmentIndex = centerSegmentIndexToEndpointIndices.size();

            const auto& endpointIndices = edgeIndexToEndpointIndices[edge_index];
            centerSegmentToEndpointPrimaryMaterialIndices[newSegmentIndex] = {
                primaryMaterialIndexByPointIndex[endpointIndices[0]],
                primaryMaterialIndexByPointIndex[endpointIndices[1]]
            };

            std::pair<int, int> newSegmentEndpointIndices;
            if (edgeIndexToFaceIndices[edge_index].size() == 1)
            {
                //Boundary edge, connect edge point and triangle point
                centerPointByIndex.push_back(edgeIndexToMidPoints[edge_index]);
                const int segmentIndex = centerPointByIndex.size() - 1;
                midpointIndexToEdgeIndex[edge_index] = segmentIndex;
                newSegmentEndpointIndices = { triangleIndexToCenterPointIndex[edgeIndexToFaceIndices[edge_index][0]], segmentIndex };
            }
            else
            {
                newSegmentEndpointIndices = { triangleIndexToCenterPointIndex[edgeIndexToFaceIndices[edge_index][0]], triangleIndexToCenterPointIndex[edgeIndexToFaceIndices[edge_index][1]] };
            }

            //Ensure consistent orientation among endpoints
            if (orientation(pointIndexToPoint.col(edgeIndexToEndpointIndices[edge_index][0]) - pointIndexToPoint.col(edgeIndexToEndpointIndices[edge_index][1]), centerPointByIndex[newSegmentEndpointIndices.first] - pointIndexToPoint.col(edgeIndexToEndpointIndices[edge_index][1])) < 0)
            {
                newSegmentEndpointIndices = { newSegmentEndpointIndices.second, newSegmentEndpointIndices.first };
            }

            centerSegmentIndexToEndpointIndices.push_back(std::move(newSegmentEndpointIndices));
        }
    }


    /*Create Fill Triangles -> Solid fill triangles for displaying areas of material type, not the contour*/
    std::vector<Eigen::Vector2f> resultingPointsByIndex;
    resultingPointsByIndex.reserve(centerPointByIndex.size() + pointIndexToPoint.cols()); //We know how big this will be
    //Join both sets of points into all the existing points
    for (const auto& centerPoint : centerPointByIndex)
    {
        resultingPointsByIndex.push_back(centerPoint);
    }
    for (const auto& pt : pointIndexToPoint.colwise())
    {
        resultingPointsByIndex.emplace_back(pt);
    }

    std::vector<std::vector<int>> resultingTriangleIndexToResultingCornerIndices;
    resultingTriangleIndexToResultingCornerIndices.reserve(numberOfTriangles * 6);
    std::vector<int> fillMats;
    fillMats.reserve(numberOfTriangles * 6);

    auto resultingTriangleCurrentSize = 0;
    /*first type of triangles : dual to mesh edges with a material change*/
    for (int edgeIndex = 0; edgeIndex < edgeIndexToEndpointIndices.size(); edgeIndex++)
    {
        //If the materials on either end of an edge don't match, there will be a triangle
        //TODO: Make all triangles connect to the edge rather than centers to centers
        if (primaryMaterialIndexByPointIndex[edgeIndexToEndpointIndices[edgeIndex][0]] != primaryMaterialIndexByPointIndex[edgeIndexToEndpointIndices[edgeIndex][1]])
        {
            std::pair<int, int> segmentEndpoints;
            if (edgeIndexToFaceIndices[edgeIndex].size() == 1) //if boundary edge edge
            {
                /*boundary edge : connect edge point and center point*/
                segmentEndpoints = { triangleIndexToCenterPointIndex[edgeIndexToFaceIndices[edgeIndex][0]], midpointIndexToEdgeIndex[edgeIndex] };
            }
            else
            {
                /*interior edge : connect two center points*/
                //We know both of these will exist b/c there is always a center point if the triangle has any material changes
                segmentEndpoints = { triangleIndexToCenterPointIndex[edgeIndexToFaceIndices[edgeIndex][0]], triangleIndexToCenterPointIndex[edgeIndexToFaceIndices[edgeIndex][1]] };
            }
            resultingTriangleIndexToResultingCornerIndices.push_back(std::vector({ segmentEndpoints.first, segmentEndpoints.second, edgeIndex + edgeIndexToEndpointIndices[edgeIndex][0] }));
            fillMats.push_back(primaryMaterialIndexByPointIndex[edgeIndexToEndpointIndices[edgeIndex][0]]);

            resultingTriangleIndexToResultingCornerIndices.push_back(std::vector({ segmentEndpoints.first, segmentEndpoints.second, edgeIndex + edgeIndexToEndpointIndices[edgeIndex][1] }));
            fillMats.push_back(primaryMaterialIndexByPointIndex[edgeIndexToEndpointIndices[edgeIndex][1]]);
        }
    }

    resultingTriangleCurrentSize = fillMats.size();
    /* second type of triangles: original mesh triangle, if there is no material change,
     or a third of the triangle, if there is some edge with no material change */
    for (int i = 0; i < triangleIndexToCornerIndices.size(); i++)
    {
        if (primaryMaterialIndexByPointIndex[triangleIndexToCornerIndices[i][1]] == primaryMaterialIndexByPointIndex[triangleIndexToCornerIndices[i][2]] && primaryMaterialIndexByPointIndex[triangleIndexToCornerIndices[i][2]] == primaryMaterialIndexByPointIndex[triangleIndexToCornerIndices[i][0]])
        {
            resultingTriangleCurrentSize += 1;
            resultingTriangleIndexToResultingCornerIndices[resultingTriangleCurrentSize] = triangleIndexToCornerIndices[i] << std::function([edgeIndex](int item) { return item + edgeIndex; });
            fillMats[resultingTriangleCurrentSize] = primaryMaterialIndexByPointIndex[triangleIndexToCornerIndices[i][0]];
        }
        else
        {
            for (int j = 0; j < 3; j++)
            {
                if (primaryMaterialIndexByPointIndex[triangleIndexToCornerIndices[i][triangle_edges[j][0]]] ==
                    primaryMaterialIndexByPointIndex[triangleIndexToCornerIndices[i][triangle_edges[j][1]]])
                {
                    resultingTriangleCurrentSize += 1;
                    resultingTriangleIndexToResultingCornerIndices[resultingTriangleCurrentSize] =
                        concat(
                            triangle_edges[j] << std::function([i, triangleIndexToCornerIndices](int in) { return triangleIndexToCornerIndices[i][in]; }) << std::function(
                                [edgeIndex](int item) { return item + edgeIndex; })
                            , { triangleIndexToCenterPointIndex[i] });
                    fillMats[resultingTriangleCurrentSize] = primaryMaterialIndexByPointIndex[triangleIndexToCornerIndices[i][triangle_edges[j][1]]];
                }
            }
        }
    }

    /*Print[ct3, fillVerts, fillTris, fillMats];*/
    resultingTriangleIndexToResultingCornerIndices = std::vector(resultingTriangleIndexToResultingCornerIndices.begin(), resultingTriangleIndexToResultingCornerIndices.begin() + resultingTriangleCurrentSize);
    fillMats = std::vector(fillMats.begin(), fillMats.begin() + resultingTriangleCurrentSize);

    return { centerPointByIndex, centerSegmentIndexToEndpointIndices, centerSegmentToEndpointPrimaryMaterialIndices, resultingPointsByIndex, resultingTriangleIndexToResultingCornerIndices, fillMats };
}
