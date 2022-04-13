/*
#include "Contour2D.h"

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

std::vector<std::vector<std::vector<Eigen::Matrix3Xf>>> getSectionContoursAll(std::vector<Eigen::Matrix3Xf>, std::vector<std::vector<std::vector<float>>>, float);
*/
