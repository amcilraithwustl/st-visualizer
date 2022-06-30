// This program is from the OpenGL Programming Guide.  It shows a robot arm
// that you can rotate by pressing the arrow keys.

#ifdef __APPLE_CC__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

// The robot arm is specified by (1) the angle that the upper arm makes
// relative to the x-axis, called shoulderAngle, and (2) the angle that the
// lower arm makes relative to the upper arm, called elbowAngle.  These angles
// are adjusted in 5 degree increments by a keyboard callback.
static int shoulderAngle = 0, elbowAngle = 0;

// Handles the keyboard event: the left and right arrows bend the elbow, the
// up and down keys bend the shoulder.
void special(int key, int, int) {
    switch (key) {
    case GLUT_KEY_LEFT: (elbowAngle += 5) %= 360; break;
    case GLUT_KEY_RIGHT: (elbowAngle -= 5) %= 360; break;
    case GLUT_KEY_UP: (shoulderAngle += 5) %= 360; break;
    case GLUT_KEY_DOWN: (shoulderAngle -= 5) %= 360; break;
    default: return;
    }
    glutPostRedisplay();
}

// wireBox(w, h, d) makes a wireframe box with width w, height h and
// depth d centered at the origin.  It uses the GLUT wire cube function.
// The calls to glPushMatrix and glPopMatrix are essential here; they enable
// this function to be called from just about anywhere and guarantee that
// the glScalef call does not pollute code that follows a call to myWireBox.
void wireBox(GLdouble width, GLdouble height, GLdouble depth) {
    glPushMatrix();
    glScalef(width, height, depth);
    glutWireCube(1.0);
    glPopMatrix();
}

// Displays the arm in its current position and orientation.  The whole
// function is bracketed by glPushMatrix and glPopMatrix calls because every
// time we call it we are in an "environment" in which a gluLookAt is in
// effect.  (Note that in particular, replacing glPushMatrix with
// glLoadIdentity makes you lose the camera setting from gluLookAt).
void display() {

    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    // Draw the upper arm, rotated shoulder degrees about the z-axis.  Note that
    // the thing about glutWireBox is that normally its origin is in the middle
    // of the box, but we want the "origin" of our box to be at the left end of
    // the box, so it needs to first be shifted 1 unit in the x direction, then
    // rotated.
    glRotatef((GLfloat)shoulderAngle, 0.0, 0.0, 1.0);
    glTranslatef(1.0, 0.0, 0.0);
    wireBox(2.0, 0.4, 1.0);

    // Now we are ready to draw the lower arm.  Since the lower arm is attached
    // to the upper arm we put the code here so that all rotations we do are
    // relative to the rotation that we already made above to orient the upper
    // arm.  So, we want to rotate elbow degrees about the z-axis.  But, like
    // before, the anchor point for the rotation is at the end of the box, so
    // we translate <1,0,0> before rotating.  But after rotating we have to
    // position the lower arm at the end of the upper arm, so we have to
    // translate it <1,0,0> again.
    glTranslatef(1.0, 0.0, 0.0);
    glRotatef((GLfloat)elbowAngle, 0.0, 0.0, 1.0);
    glTranslatef(1.0, 0.0, 0.0);
    wireBox(2.0, 0.4, 1.0);

    glPopMatrix();
    glFlush();
}

// Handles the reshape event by setting the viewport so that it takes up the
// whole visible region, then sets the projection matrix to something reason-
// able that maintains proper aspect ratio.
void reshape(GLint w, GLint h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(65.0, GLfloat(w) / GLfloat(h), 1.0, 20.0);
}

// Perfroms application specific initialization: turn off smooth shading,
// sets the viewing transformation once and for all.  In this application we
// won't be moving the camera at all, so it makes sense to do this.
void init() {
    glShadeModel(GL_FLAT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(1, 2, 8, 0, 0, 0, 0, 1, 0);
}

// Initializes GLUT, the display mode, and main window; registers callbacks;
// does application initialization; enters the main event loop.
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowPosition(80, 80);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Robot Arm");
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutSpecialFunc(special);
    init();
    glutMainLoop();
}

// #include <string>
// #include <fstream>
// #include "JSONParser.h"
// #include "UtilityFunctions.h"
// #include "ImportFunctions.h"
// #include "tetgen1.6.0/tetgen.h"
// #include "Contour3D.h"
// using namespace nlohmann;
// //https://wias-berlin.de/software/tetgen/
//
// //https://www.youtube.com/watch?v=A1LqGsyl3C4
//
//
// int main(int argc, char* argv[])
// {
// 	using json = nlohmann::json;
// 	int i = 0;
//
// 	auto jsonToMatrix = [](const json& source)
// 	{
// 		//This should only be used for testing. Not logic safe.
// 		std::vector<Eigen::Vector3f> a;
// 		a.reserve(source.size());
// 		for (int i = 0; i < source.size(); i++)
// 		{
// 			a.emplace_back(
// 				source[i][0],
// 				source[i][1],
// 				source[i][2]
// 			);
// 		}
// 		return a;
// 	};
//
// 	auto jsonToVector = [](const json& source)
// 	{
// 		std::vector<std::vector<float>> ret;
// 		for (auto row : source)
// 		{
// 			std::vector<float> temp;
// 			for (auto item : row)
// 			{
// 				temp.push_back(item);
// 			}
// 			ret.push_back(temp);
// 		}
// 		return ret;
// 	};
//
// 	auto jsonToTets = [](const json& source)
// 	{
// 		std::vector<std::vector<int>> ret;
// 		for (auto row : source)
// 		{
// 			std::vector<int> temp;
// 			for (auto item : row)
// 			{
// 				temp.push_back(static_cast<int>(item) - 1);//Subtract 1 b/c mathematica indices start a 1
// 			}
// 			ret.push_back(temp);
// 		}
// 		return ret;
// 	};
// 	// read a JSON file
// 	std::ifstream file("C:\\Users\\Aiden McIlraith\\Documents\\GitHub\\st-visualizer\\UnitTest\\singleContour3DTest.json");
// 	auto a = file.is_open();
// 	json j2 = json::parse(file);
// 	json ret2 = json::array();
// 	for (auto& j : j2) {
// 		std::cout << std::endl << std::endl << "~~ Begin New Evaluation ~~" << std::endl;
// 		auto pts = jsonToMatrix(j[0]);
//         auto vals = jsonToVector(j[1]);
//         auto tets = jsonToTets(j[2]);
// 		j.clear();
//
//         auto [verts, segs, segmats] = contourTetMultiDC(pts, tets, vals);
// 		auto ctrs = getContourAllMats3D(verts, segs, segmats, vals[0].size(), 0.04);
// 		json ret = json::array();
// 		ret.push_back(pts);
// 		ret.push_back(vals);
// 		ret.push_back(tets);
// 		ret.push_back(verts);
// 		ret.push_back(segs);
// 		ret.push_back(segmats);
// 		ret.push_back(ctrs);
// 		ret.push_back({});
// 		ret2.push_back(ret);
// 	}
// 	std::ofstream f("C:\\Users\\Aiden McIlraith\\Documents\\GitHub\\st-visualizer\\UnitTest\\singleContour3DResults.json");
// 	f << ret2;
//
// 	return 0;
// }