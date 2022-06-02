#pragma once
#include "UtilityFunctions.h"

#include "tetgen1.6.0/tetgen.h"

inline void orderTets()
{
    
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