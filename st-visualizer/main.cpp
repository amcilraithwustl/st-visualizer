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

// The cube has opposite corners at (0,0,0) and (1,1,1), which are black and
// white respectively.  The x-axis is the red gradient, the y-axis is the
// green gradient, and the z-axis is the blue gradient.  The cube's position
// and colors are fixed.
namespace Cube {

    const int NUM_VERTICES = 8;
    const int NUM_FACES = 6;

    GLint vertices[NUM_VERTICES][3] = {
      {0, 0, 0}, {0, 0, 1}, {0, 1, 0}, {0, 1, 1},
      {1, 0, 0}, {1, 0, 1}, {1, 1, 0}, {1, 1, 1} };

    GLint faces[NUM_FACES][4] = {
      {1, 5, 7, 3}, {5, 4, 6, 7}, {4, 0, 2, 6},
      {3, 7, 6, 2}, {0, 1, 3, 2}, {0, 4, 5, 1} };

    GLfloat vertexColors[NUM_VERTICES][3] = {
      {0.0, 0.0, 0.0}, {0.0, 0.0, 1.0}, {0.0, 1.0, 0.0}, {0.0, 1.0, 1.0},
      {1.0, 0.0, 0.0}, {1.0, 0.0, 1.0}, {1.0, 1.0, 0.0}, {1.0, 1.0, 1.0} };

    void draw() {
        glBegin(GL_QUADS);
        for (int i = 0; i < NUM_FACES; i++) {
            for (int j = 0; j < 4; j++) {
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
void timer(int v) {
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
void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, GLfloat(w) / GLfloat(h), 0.5, 40.0);
    glMatrixMode(GL_MODELVIEW);
}

// Application specific initialization:  The only thing we really need to do
// is enable back face culling because the only thing in the scene is a cube
// which is a convex polyhedron.
void init() {
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}


#include "JSONParser.h"
#include "UtilityFunctions.h"
#include "tetgen1.6.0/tetgen.h"
#include "Contour3D.h"
using namespace nlohmann;
//https://wias-berlin.de/software/tetgen/

//https://www.youtube.com/watch?v=A1LqGsyl3C4


void drawContour(std::vector<Eigen::Vector3f> contour)
{
    glBegin(GL_POLYGON);
    for (int i = 0; i < contour.size(); i++) {
        glColor3fv(Cube::vertexColors[i%Cube::NUM_VERTICES]);
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
void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    // Cube::draw();

    //Render both sides of the polygon
    glDisable(GL_CULL_FACE);


    drawContour({ {0,0,0},{1,0,0},{1,1,0},{0,1,0},{0,1,1},{0,0,1},{1, 0, 1} });
    glFlush();
    glutSwapBuffers();
}

//Useful opengl examples: https://cs.lmu.edu/~ray/notes/openglexamples/
int main(int argc, char* argv[])
{
	constexpr auto n = 50;
    auto rand_pts = Eigen::Matrix3Xf::Random(3, n);
	std::vector<Eigen::Vector3f> pts;
	pts.reserve(n);
	for (int i = 0; i < rand_pts.cols(); i++)
	{
		const auto temp = rand_pts.col(i);
		pts.emplace_back(temp);
	}
	std::vector<std::vector<float>> vals;
	vals.reserve(n);
	for (int i = 0; i < rand_pts.cols(); i++)
	{
        constexpr auto m = 5;
        vals.emplace_back();
		const auto v = Eigen::VectorXf::Random(m);
		for (int i = 0; i < m; i++)
		{
			vals[vals.size() - 1].push_back(v(i));
		}
	}
	std::vector<std::vector<int>> tets;
	{
		tetgenio out;
		tetralizeMatrix(rand_pts, out);
		tets = tetgenToTetVector(out);
	}

	auto [verts, segs, segmats] = contourTetMultiDC(pts, tets, vals);
	auto ctrs = getContourAllMats3D(verts, segs, segmats, vals[0].size(), 0.04);

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(500, 500);
    glutCreateWindow("The RGB Color Cube");
    glutReshapeFunc(reshape);
    glutTimerFunc(100, timer, 0);
    glutDisplayFunc(display);
    init();
    glutMainLoop();

	return 0;
}