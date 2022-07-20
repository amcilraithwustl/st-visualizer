// This program is from the OpenGL Programming Guide.  It shows a robot arm
// that you can rotate by pressing the arrow keys.

// This program is a flyby around the RGB color cube.  One intersting note
// is that because the cube is a convex polyhedron and it is the only thing
// in the scene, we can render it using backface culling only. i.e., there
// is no need for a depth buffer.

#ifdef __APPLE_CC__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <cmath>
#include <fstream>

#include "JSONParser.h"
#include "UtilityFunctions.h"
#include "tetgen1.6.0/tetgen.h"
#include "Contour3D.h"
#include "ImportFunctions.h"
using namespace nlohmann;


//https://wias-berlin.de/software/tetgen/
//https://www.youtube.com/watch?v=A1LqGsyl3C4
//Useful opengl examples: https://cs.lmu.edu/~ray/notes/openglexamples/


//Explore electron as front-end option vs C++ w/ QT

//GUI with QT (if C++)

// The cube has opposite corners at (0,0,0) and (1,1,1), which are black and
// white respectively.  The x-axis is the red gradient, the y-axis is the
// green gradient, and the z-axis is the blue gradient.  The cube's position
// and colors are fixed.
namespace Cube
{
    const int NUM_VERTICES = 8;
    const int NUM_FACES = 6;

    GLint vertices[NUM_VERTICES][3] = {
        {0, 0, 0},
        {0, 0, 1},
        {0, 1, 0},
        {0, 1, 1},
        {1, 0, 0},
        {1, 0, 1},
        {1, 1, 0},
        {1, 1, 1}
    };

    GLint faces[NUM_FACES][4] = {
        {1, 5, 7, 3},
        {5, 4, 6, 7},
        {4, 0, 2, 6},
        {3, 7, 6, 2},
        {0, 1, 3, 2},
        {0, 4, 5, 1}
    };

    GLfloat vertexColors[NUM_VERTICES][3] = {
        {0.0, 0.0, 0.0},
        {0.0, 0.0, 1.0},
        {0.0, 1.0, 0.0},
        {0.0, 1.0, 1.0},
        {1.0, 0.0, 0.0},
        {1.0, 0.0, 1.0},
        {1.0, 1.0, 0.0},
        {1.0, 1.0, 1.0}
    };

    void draw()
    {
        glBegin(GL_QUADS);
        for(int i = 0; i < NUM_FACES; i++)
        {
            for(int j = 0; j < 4; j++)
            {
                glColor3fv((GLfloat*)&vertexColors[faces[i][j]]);
                glVertex3iv((GLint*)&vertices[faces[i][j]]);
            }
        }
        glEnd();
    }
}


// We'll be flying around the cube by moving the camera along the orbit of the
// curve u->(8*cos(u), 7*cos(u)-1, 4*cos(u/3)+2).  We keep the camera looking
// at the center of the cube (0.5, 0.5, 0.5) and vary the up vector to achieve
// a weird tumbling effect.
void timer(int v)
{
    static GLfloat u = 0.0;
    u += 0.01;
    glLoadIdentity();
    gluLookAt(8 * cos(u), 7 * cos(u) - 1, 4 * cos(u / 3) + 2, .5, .5, .5, cos(u), 1, 0);
    glutPostRedisplay();
    glutTimerFunc(1000 / 60.0, timer, v);
}

// When the window is reshaped we have to recompute the camera settings to
// match the new window shape.  Set the viewport to (0,0)-(w,h).  Set the
// camera to have a 60 degree vertical field of view, aspect ratio w/h, near
// clipping plane distance 0.5 and far clipping plane distance 40.
void reshape(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, static_cast<GLfloat>(w) / static_cast<GLfloat>(h), 0.5, 40.0);
    glMatrixMode(GL_MODELVIEW);
}

// Application specific initialization:  The only thing we really need to do
// is enable back face culling because the only thing in the scene is a cube
// which is a convex polyhedron.
void init()
{
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}


int index = 0;

void drawContour(const std::vector<Eigen::Vector3f>& contour)
{
    glBegin(GL_POLYGON);
    for(int i = 0; i < contour.size(); i++)
    {
        glColor3fv(Cube::vertexColors[index % Cube::NUM_VERTICES]);
        glVertex3fv(contour[i].data());
    }
    glEnd();
}

// Display and Animation. To draw we just clear the window and draw the cube.
// Because our main window is double buffered we have to swap the buffers to
// make the drawing visible. Animation is achieved by successively moving our
// camera and drawing. The function nextAnimationFrame() moves the camera to
// the next point and draws. The way that we get animation in OpenGL is to
// register nextFrame as the idle function; this is done in main().

std::vector<std::pair<std::vector<Eigen::Vector3f>, std::vector<std::vector<int>>>>* contours = nullptr;
std::vector<Eigen::Vector3f>* points = nullptr;

void drawPoints()
{
    glBegin(GL_POINTS);
    glEnable(GL_POINT_SIZE);
    for(const auto& point : *points)
    {
        glColor3fv(Cube::vertexColors[7]);
        glVertex3fv(point.data());
    }
    glEnd();
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    // Cube::draw();

    //Render both sides of the polygon
    // glDisable(GL_CULL_FACE);

    index = 0;
    for(const auto& [vertices, segments] : *contours)
    {
        index++;

        for(auto& seg : segments)
        {
            std::vector<Eigen::Vector3f> temp;
            temp.reserve(seg.size());
            for(auto& index : seg)
            {
                temp.push_back(vertices[index]);
            }
            drawContour(temp);
        }
    }
    drawPoints();
    glFlush();
    glutSwapBuffers();
}
std::vector<std::pair<std::vector<Eigen::Vector3f>, std::vector<std::vector<int>>>>
getVolumeContours(const Eigen::Matrix3Xf& pts, std::vector<std::vector<float>> vals, float shrink)
{
    const auto nmat = vals[0].size();
    tetgenio reg;
    tetralizeMatrix(pts, reg);
    const auto tets = tetgenToTetVector(reg);
    std::vector<Eigen::Vector3f> pts_vector;
    pts_vector.reserve(pts.cols());
    //TODO: Remove the need for the data transform again by using Eigen::Matrix rather than a std::vector of Eigen::Vector
    for (auto& pt : pts.colwise())
    {
        pts_vector.push_back(pt);
    }
    auto [verts, segs, segmats] = contourTetMultiDC(pts_vector, tets, vals);
    return getContourAllMats3D(
        verts, segs, segmats, nmat, shrink);
}


Eigen::Matrix3Xf concatMatrixes(const std::vector<Eigen::Matrix3Xf>& input)
{
    unsigned int sum = 0;
    for (auto& layer : input)
    {
        sum += layer.cols();
    }
    Eigen::Matrix3Xf result(3, sum);
    unsigned int i = 0;
    for (const auto& layer : input)
    {
        for (const auto& pt : layer.colwise())
        {
            result.col(i) = pt;
            i++;
        }
    }
    return result;
}

template <typename T>
std::vector<T> flatten(const std::vector<std::vector<T>>& input)
{
    unsigned int sum = 0;
    for (auto& layer : input)
    {
        sum += layer.size();
    }
    std::vector<T> result;
    result.reserve(sum);
    unsigned int i = 0;
    for (const auto& layer : input)
    {
        for (const auto& pt : layer)
        {
            result.push_back(pt);
        }
    }
    return result;
}


int main(int argc, char* argv[])
{
    {
        constexpr float shrink = 0.04;
        std::vector<std::string> sliceNames({ "NMK_F_U1", "NMK_F_U2", "NMK_F_U3", "NMK_F_U4" });
        std::vector<unsigned> featureCols({ 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 });
        const auto alignmentValues = importAlignments(
            "C:/Users/Aiden McIlraith/Documents/GitHub/st-visualizer/NMK_F_transformation_pt_coord.csv");
        const auto results = loadTsv(
            "C:/Users/Aiden McIlraith/Documents/GitHub/st-visualizer/NMK_20201201_cell_type_coord_allspots.tsv",
            sliceNames,
            1,
            2,
            std::pair<unsigned, unsigned>(3, 4),
            5,
            std::vector<unsigned>({ 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 }),
            60,
            alignmentValues
        );

        auto [ctrs2dVals, tris2dVals] = getSectionContoursAll(results.slices, results.values, shrink);
        auto [ctrs2dclusters, tris2dclusters] = getSectionContoursAll(results.slices, results.clusters, shrink);


        auto allpts = concatMatrixes(results.slices);
        auto ctrs3dVals = getVolumeContours(allpts, flatten<std::vector<float>>(results.values), shrink);
        auto ctrs3dClusters = getVolumeContours(allpts, flatten<std::vector<float>>(results.clusters), shrink);
        auto ptClusIndex = results.clusters << std::function([](std::vector<std::vector<float>> layer)
            {
                return layer << std::function(getMaxPos);
            });
        auto ptValIndex = results.values << std::function([](std::vector<std::vector<float>> layer)
            {
                return layer << std::function(getMaxPos);
            });
        auto slices = results.slices << std::function([](const Eigen::Matrix3Xf& layer)
            {
                std::vector<Eigen::Vector3f> temp;
                temp.reserve(layer.cols());
                for (const auto& pt : layer.colwise())
                {
                    temp.emplace_back(pt);
                }
                return temp;
            });

        auto convertCtrs = [](
            std::vector<std::vector<std::pair<
            std::vector<Eigen::Matrix<float, 3, 1, 0>>, std::vector<std::pair<int, int>>>>>& ctrs2dVals)
        {
            json ctrs2dValsJson = json::array();
            for (auto& ctrSlice : ctrs2dVals)
            {
                json ctrJson = json::array();
                for (auto& ctr : ctrSlice)
                {
                    json temp = json::array();
                    temp.push_back(ctr.first);
                    for (auto& i : ctr.second)
                    {
                        // i.first++;
                        // i.second++;
                    }
                    temp.push_back(ctr.second);
                    ctrJson.push_back(temp);
                }

                ctrs2dValsJson.push_back(ctrJson);
            }

            return ctrs2dValsJson;
        };

        auto convertTris = [](std::vector<std::tuple<
            std::vector<Eigen::Matrix<float, 3, 1, 0>>,
            std::vector<std::vector<int>>,
            std::vector<int>
            >>&tris2dVals)
        {
            json tris2dValsJson = json::array();
            for (auto& tris : tris2dVals)
            {
                json a = json::array();
                {
                    json b = json::array();
                    for (auto& elem : std::get<0>(tris))
                    {
                        b.push_back(std::vector(elem.data(), elem.data() + elem.rows()));
                    }
                    a.push_back(b);
                }

                auto& triangles = std::get<1>(tris);
                for (auto& tri : triangles)
                {
                    for (auto& ind : tri)
                    {
                        // ind++;
                    }
                }
                a.push_back(triangles);

                auto& materials = std::get<2>(tris);
                for (auto& mat : materials) { mat++; }
                a.push_back(materials);

                tris2dValsJson.push_back(a);
            }

            return tris2dValsJson;
        };

        auto convert3D = [](
            std::vector<std::pair<std::vector<Eigen::Vector3f>, std::vector<std::vector<int>>>>& ctrs3d)
        {
            json ctrs3dJson = json::array();
            for (auto& ctr : ctrs3d)
            {
                json a = json::array();

                a.push_back(ctr.first);
                auto& segs = ctr.second;
                for (auto& a : segs)
                {
                    for (auto& b : a)
                    {
                        // b++;
                    }
                }
                a.push_back(segs);

                ctrs3dJson.push_back(a);
            }

            return ctrs3dJson;
        };

        json ret = json::object();
        ret["nat"]=(results.values[0][0].size()); //nMat,
        ret["shrink"]=(shrink); //shrink,
        ret["clusters"]=(results.clusters); //clusters,
        ret["slices"]=(slices); //slices,
        ret["ptClusIndex"]=(ptClusIndex); //ptClusIndex,
        ret["ctrs2Dvals"]=(convertCtrs(ctrs2dVals)); //ctrs2Dvals,
        ret["ctrs3Dvals"]=(convert3D(ctrs3dVals)); //ctrs3Dvals,
        ret["featureNames"]=(results.names); //featureNames,
        ret["ptValIndex"]=(ptValIndex); //ptValIndex,
        ret["tris2Dvals"]=(convertTris(tris2dVals)); //tris2Dvals
        ret["ctrs2Dclusters"]=(convertCtrs(ctrs2dclusters)); //ctrs2Dclusters,
        ret["ctrs3Dclusters"]=(convert3D(ctrs3dClusters)); //ctrs3Dclusters,
        ret["nClusters"]=(results.clusters[0][0].size()); //nClusters,
        ret["tris2Dclusters"]=(convertTris(tris2dclusters)); //tris2Dclusters,
        ret["featureCols"]=(featureCols); //featureCols,
        ret["sliceNames"]=(sliceNames); //sliceNames
        ret["values"]=(results.values);

        std::ofstream f("C:\\Users\\Aiden McIlraith\\Documents\\GitHub\\st-visualizer\\st-visualizer-electron\\imports\\static\\integrationTest.json");
        f << ret;
    }
}
