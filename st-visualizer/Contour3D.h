#pragma once
#include "UtilityFunctions.h"

#include "tetgen1.6.0/tetgen.h"

inline std::pair<std::vector<int>,std::vector<int>> orderTets(std::pair<int, int> edges, std::vector<std::vector<int>> tets)
{
    std::vector<std::vector<int>> nonEdgeCornersByTet; //This should have two non-edge corners per tet
    {
        nonEdgeCornersByTet.reserve(tets.size());

        for (const auto& tet : tets)
        {
            std::vector<int> complement;
            for (const auto& corner : tet)
            {
                if (corner != edges.first && corner != edges.second) complement.push_back(corner);
            }
            nonEdgeCornersByTet.push_back(std::move(complement));
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

    std::vector<int> uniqueCorners; //There will be either 0 or 2 unique corners
    std::vector tetsByCorner(nonEdgeCornersSet[nonEdgeCornersSet.size()-1]+1, std::vector<std::pair<int, int>>()); //Which tet, which index in tet
    {
        for (const auto& corner : nonEdgeCornersSet)
        {
            auto& tetEntry = tetsByCorner[corner];
            for (int i = 0; i < nonEdgeCornersByTet.size(); i++)
            {
                for (int j = 0; j < nonEdgeCornersByTet[i].size(); j++)
                {
                    if (nonEdgeCornersByTet[i][j] == corner)
                        tetEntry.emplace_back(i, j);
                }
            }

            if (tetEntry.size() == 1)//If the corner isn't shared by any other tets
            {
                uniqueCorners.push_back(corner);
            }
        }
    }

    std::vector<int> orderedTets;
    int endPoint;
    int nextCorner;
    //If there are no boundary spaces
    if(uniqueCorners.empty())
    {
        //If there are no unique corners, just start with the first tet and go from there
        orderedTets.push_back(0);
        nextCorner = nonEdgeCornersByTet[0][1];//start with the first tet's second point
        endPoint = nonEdgeCornersByTet[0][0];//And end with its first point
    }
    //If there are boundary spaces
    else
    {
        //We know the associated set of tets will have exactly one entry
        const auto& temp = tetsByCorner[uniqueCorners[0]][0];//The tet associated with the first unique corner
        orderedTets.push_back(temp.first);
        nextCorner = nonEdgeCornersByTet[temp.first][1 - temp.second];//Start with the other corner in that tet
        endPoint = uniqueCorners[1];//End with the other unique corner
    }

    //Run through all the connected tets to order them correctly
    while (nextCorner != endPoint)
    {
        auto tetEntry = tetsByCorner[nextCorner];

        //If we have already looked at the first tet in the entry
        if (tetEntry[0].first == orderedTets[orderedTets.size() - 1])
        {
            orderedTets.push_back(tetEntry[1].first);//Push back the second tet index
            nextCorner = nonEdgeCornersByTet[tetEntry[1].first][1 - tetEntry[1].second];//The new next is the other corner in that tet
        }
        else
        {
            orderedTets.push_back(tetEntry[0].first);//push back the tet index from the first entry
            nextCorner = nonEdgeCornersByTet[tetEntry[0].first][1 - tetEntry[0].second]; //The new next is the other corner in that tet
        }
    }

    return { orderedTets, uniqueCorners };
}

inline void contourTetMultiDC()
{
    
}

inline void tetralizeMatrix(const Eigen::Matrix3Xf& pts, tetgenio& out)
{

    tetgenio in;

    in.firstnumber = 0;//Arrays start at 0

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