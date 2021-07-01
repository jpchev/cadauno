/*
    This file is part of CADauno.
    Copyright (C) 2009 Giampaolo Capelli

    CADauno is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 2 of the License.

    CADauno is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with CADauno; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

//use a low level definition for PI, it's only for camera
#define PI_VALUE 3.14

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "include/glui.h"
#include "evaluator.h"
#include "serializator.h"
#include "columnsOperations.h"
#include "entities.h"

#ifdef _WIN32
#define STDCALL __stdcall
#else
#define STDCALL
#endif

/*
in the model window:
-the arrow keys move the camera view point (eye) in spherical coordinates
-keys + e - modify the sperical coordinates radius
-keys a, z, s, x, d, c modify the camera looked point (reference)
-key r restores the default state
*/

/*
state variabiles for camera angles and radius in spherical coordinates
       |y
       |
       |  /P
       | /
       |/_________x
      /O
     /
    /z
theta: angle between the y axis and the segment PO
phi: angle between the z axis and the x axis in the zx plane
*/
GLfloat phi = 0.0, theta = PI_VALUE / 2.0, rho = 3.9;

//main window dimensions
#define WIDTH 640
#define HEIGHT 480

//current windows dimension
int Window_Width = WIDTH;
int Window_Height = HEIGHT;

int Window_Width_section = WIDTH / 2;
int Window_Height_section = HEIGHT / 2;

int main_window_id = 0, curveWindow = 0;
int selected_column = 0;
int destination_column_copy = 0;

int display_skinning = 0;

GLfloat barycentre[3];

GLUnurbs *nurbs = NULL;

/////////////////////////////////////////////////////////////////////////////
///////////////////////// variables for the GLUI interface //////////////////
/////////////////////////////////////////////////////////////////////////////
GLUI *glui = NULL;

char filename[sizeof(GLUI_String)];
char background[sizeof(GLUI_String)];
char background_temp[sizeof(GLUI_String)];

GLfloat transl_x[1], transl_y[1], transl_z[1], transl_zy[2];

GLfloat old_transl_x[1], old_transl_y[1], old_transl_z[1], old_transl_zy[2];

GLUI_Translation *x_translation = NULL, *y_translation = NULL, *z_translation = NULL, *zy_translation = NULL;
GLUI_Rotation *rotation = NULL;

GLfloat z_rotation = 0.0;
GLUI_Spinner *z_rot_spinner = NULL, *spinner_multiplicity = NULL, *spinner_destination = NULL;

GLfloat rotation_data[16] = {1.0, 0.0, 0.0, 0.0,
							 0.0, 1.0, 0.0, 0.0,
							 0.0, 0.0, 1.0, 0.0,
							 0.0, 0.0, 0.0, 1.0};
/////////////////////////////////////////////////////////////////////////////
/////////////////// end of variables for the GLUI interface  ////////////////
/////////////////////////////////////////////////////////////////////////////

//declaration of other functions
void display();
void displaySection(void);
void find_barycentre(GLfloat *);
void releaseResource();
void createNewSurf();
void guiInit(int argc, char **argv);

point3d getSectionEye();
point3d getSectionReference();

///////////////////////////////////////////////////////////////////////
//////////////////////////// control points ///////////////////////////
///////////////////////////////////////////////////////////////////////
int NCP_S = -1;
int NCP_T = -1;

int S_ORDER = -1;

int T_ORDER = -1;

int S_NUMKNOTS = NCP_S + S_ORDER;
int T_NUMKNOTS = NCP_T + T_ORDER;

GLfloat *skinning_ctrlpoints = NULL;

GLfloat *ctrlpoints = NULL;

GLfloat *sknots = NULL;

GLfloat *skinning_tknots = NULL;
GLfloat *tknots = NULL;

int multiplicity = -1;

int indexSelectedCtrlPoint = -1;
GLfloat weightSelectedCtrlPoint = -1.0;
///////////////////////////////////////////////////////////////////////
/////////////////////////// end of control points /////////////////////
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
////// variables for the evaluation of points on the NURBS surface ////
///////////////////////////////////////////////////////////////////////

GLfloat u = 0.0, v = 0.0, u_inc = 0.01, v_inc = 0.01;

///////////////////////////////////////////////////////////////////////
// end of variables for the evaluation of points on the NURBS surface /
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
///////////////////////////// objects colour //////////////////////////
///////////////////////////////////////////////////////////////////////
void set_material_color(GLfloat *mat_ambient,
						GLfloat *mat_specular,
						GLfloat *mat_diffuse,
						GLfloat *mat_shininess)
{
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
	if (mat_shininess != NULL)
		glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
}

void set_color(GLfloat *color,
			   GLfloat *shininess = NULL)
{
	set_material_color(color, color, color, shininess);
}

void set_nurbs_color(void)
{
	GLfloat mat_ambient[] =
		{0.3, 0.3, 0.5, 1.0};
	GLfloat mat_diffuse[] =
		{0.3, 0.3, 0.5, 1.0};
	GLfloat mat_specular[] =
		{0.3, 0.3, 0.3, 1.0};
	GLfloat mat_shininess[] =
		{50.0};

	set_material_color(mat_ambient,
					   mat_diffuse,
					   mat_specular,
					   mat_shininess);
}

void set_lines_color(void)
{
	GLfloat mat_ambient[] =
		{0.2, 0.5, 0.6, 1.0};
	GLfloat mat_diffuse[] =
		{0.2, 0.5, 0.7, 1.0};
	GLfloat mat_specular[] =
		{0.4, 0.5, 0.4, 1.0};
	GLfloat mat_shininess[] =
		{30.0};
	set_material_color(mat_ambient,
					   mat_diffuse,
					   mat_specular,
					   mat_shininess);
}

void set_selected_color(void)
{
	GLfloat mat_ambient[] =
		{0.8, 0.3, 0.6, 1.0};
	GLfloat mat_diffuse[] =
		{0.8, 0.3, 0.7, 1.0};
	GLfloat mat_specular[] =
		{0.8, 0.2, 0.4, 1.0};
	GLfloat mat_shininess[] =
		{30.0};
	set_material_color(mat_ambient,
					   mat_diffuse,
					   mat_specular,
					   mat_shininess);
}

void set_skinning_ctrlpts_color()
{
	GLfloat mat_ambient[] =
		{1.0, 0.0, 0.0, 1.0};
	GLfloat mat_diffuse[] =
		{1.0, 0.0, 0.0, 1.0};
	GLfloat mat_specular[] =
		{1.0, 0.0, 0.0, 1.0};
	GLfloat mat_shininess[] =
		{30.0};
	set_material_color(mat_ambient,
					   mat_diffuse,
					   mat_specular,
					   mat_shininess);
}
///////////////////////////////////////////////////////////////////////
///////////////////// colours object //////////////////////////////////
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
////////////////////// callback for the main window ///////////////////
///////////////////////////////////////////////////////////////////////
void drawPlanes()
{
	int j, k;
	for (k = 0; k < NCP_T; k++)
	{

		if (k == selected_column)
			set_selected_color();
		else
			set_lines_color();

		glBegin(GL_LINE_STRIP);

		for (j = 0; j < NCP_S; j++)
			glVertex4fv(ctrlpoints + (NCP_S * k + j) * 4);

		glEnd();

	} //for

	set_lines_color();

	//find the barycentre of the selected column
	find_barycentre(barycentre);

	if (display_skinning != 0)
	{
		for (k = 0; k < NCP_T; k++)
		{

			glBegin(GL_LINE_STRIP);

			for (j = 0; j < NCP_S; j++)
				glVertex4fv(skinning_ctrlpoints + (NCP_S * k + j) * 4);

			glEnd();

		} //for
	}

} //drawPlanes

void nurbsError(GLenum errorCode)
{
	const GLubyte *estring;
	estring = gluErrorString(errorCode);
	fprintf(stdout, "Nurbs Error: %s\n", estring);
	//exit (0);
}

int nurbsDisplayModeInt = 0;
GLfloat nurbsDisplayMode = GLU_OUTLINE_POLYGON;
int showCtrlPtsPlanes = 0;

void renderBitmapString(
	float x,
	float y,
	float z,
	void *font,
	char *string)
{
	char *c;
	glRasterPos3f(x, y, z);
	for (c = string; *c != '\0'; c++)
	{
		glutBitmapCharacter(font, *c);
	}
}

GLfloat *multiple_tknots = NULL, *multiple_ctrlpoints = NULL;
int num_multiple_ctrlpoints, num_multiple_tknots;

void initMultipleCtrlPoints()
{
	int i = 0, j = 0, k = 0;
	if (multiple_tknots != NULL)
	{
		free(multiple_tknots);
	}
	if (multiple_ctrlpoints != NULL)
	{
		free(multiple_ctrlpoints);
	}
	multiple_tknots =
		(GLfloat *)malloc(sizeof(GLfloat) * (NCP_T * multiplicity + T_ORDER));

	multiple_ctrlpoints =
		(GLfloat *)malloc(sizeof(GLfloat) * (NCP_T * multiplicity * NCP_S * 4));

	num_multiple_ctrlpoints = (NCP_T * multiplicity) * NCP_S;
	num_multiple_tknots = NCP_T * multiplicity + T_ORDER;

	for (i = 0; i < num_multiple_tknots; i++)
	{
		if (i < T_ORDER)
			multiple_tknots[i] = 0.0;
		else
		{
			if (i >= (num_multiple_tknots - T_ORDER))
				multiple_tknots[i] = 1.0;
			else
				multiple_tknots[i] =
					((float)(((float)(i - T_ORDER + 1)) /
							 ((float)(num_multiple_tknots - T_ORDER * 2 + 1))));
		} //else
	}	  //for

	for (i = 0; i < NCP_T; i++)
	{
		for (k = 0; k < multiplicity; k++)
		{
			for (j = 0; j < NCP_S * 4; j++)
			{
				multiple_ctrlpoints[NCP_S * 4 * i * multiplicity +
									j + k * NCP_S * 4] =
					ctrlpoints[NCP_S * 4 * i + j];
			}
		}
	}
}

void draw()
{

	if (multiple_tknots == NULL || multiple_ctrlpoints == NULL)
		initMultipleCtrlPoints();

	if (nurbs == NULL)
	{
		nurbs = gluNewNurbsRenderer();
		gluNurbsCallback(nurbs, GLU_ERROR, (void(STDCALL *)(void))nurbsError);
	}

	set_nurbs_color();

	gluNurbsProperty(nurbs,
					 GLU_DISPLAY_MODE,
					 nurbsDisplayMode);
	gluNurbsProperty(nurbs, GLU_CULLING, GL_TRUE);

	gluBeginSurface(nurbs);

	GLint t_numknots = T_NUMKNOTS;	   //default
	GLfloat *_tknots = tknots;		   //default
	GLfloat *_ctrlpoints = ctrlpoints; //default

	if (multiplicity > 1)
	{
		t_numknots = num_multiple_tknots;
		_tknots = multiple_tknots;
		_ctrlpoints = multiple_ctrlpoints;
	}

	if (display_skinning != 0)
	{
		//show skinning surface
		gluNurbsSurface(
			nurbs,
			S_NUMKNOTS, sknots,
			T_NUMKNOTS, skinning_tknots,
			4, 4 * NCP_S,
			skinning_ctrlpoints,
			S_ORDER, T_ORDER,
			GL_MAP2_VERTEX_4);
	}
	else
	{
		gluNurbsSurface(
			nurbs,
			S_NUMKNOTS, sknots,
			t_numknots, _tknots,
			4, 4 * NCP_S,
			_ctrlpoints,
			S_ORDER, T_ORDER,
			GL_MAP2_VERTEX_4);
	}

	gluEndSurface(nurbs);

	if (showCtrlPtsPlanes == 1)
	{
		drawPlanes();
		//draws control points
		glBegin(GL_POINTS);
		for (int i = 0; i < NCP_T; i++)
		{
			for (int j = 0; j < NCP_S; j++)
			{

				if (i * NCP_S * 4 + j * 4 == indexSelectedCtrlPoint)
					set_selected_color();

				glVertex4fv(ctrlpoints + i * NCP_S * 4 + j * 4);

				if (i * NCP_S * 4 + j * 4 == indexSelectedCtrlPoint)
					set_lines_color();
			}
		}
		glEnd();

		//draws skinning control points
		if (display_skinning != 0)
		{
			set_skinning_ctrlpts_color();
			glBegin(GL_POINTS);
			for (int i = 0; i < NCP_T; i++)
			{
				for (int j = 0; j < NCP_S; j++)
				{

					if (i * NCP_S * 4 + j * 4 == indexSelectedCtrlPoint)
						set_selected_color();

					glVertex4fv(skinning_ctrlpoints + i * NCP_S * 4 + j * 4);

					if (i * NCP_S * 4 + j * 4 == indexSelectedCtrlPoint)
						set_lines_color();
				}
			}
			glEnd();
		}
	}

	//draws camera reference and eye
	point3d sectionReference = getSectionReference(),
			sectionEye = getSectionEye();
	glPointSize(15.0);
	glBegin(GL_POINTS);
	glVertex3f(sectionReference.x, sectionReference.y, sectionReference.z);
	glVertex3f(sectionEye.x, sectionEye.y, sectionEye.z);
	glEnd();
	glBegin(GL_LINE_STRIP);
	glVertex3f(sectionReference.x, sectionReference.y, sectionReference.z);
	glVertex3f(sectionEye.x, sectionEye.y, sectionEye.z);
	glEnd();
	renderBitmapString(sectionEye.x, sectionEye.y, sectionEye.z, GLUT_BITMAP_TIMES_ROMAN_24, "section cam eye");
	renderBitmapString(sectionReference.x, sectionReference.y, sectionReference.z, GLUT_BITMAP_TIMES_ROMAN_24, "section cam ref");

} //draw

void aspectRatio(int w, int h)
{
	float ratio;
	// Compute new aspect ratio
	if (h == 0)
		h = 1;
	ratio = 1.0 * w / h;
	Window_Width = w;
	Window_Height = h;

	// Reset the coordinate system before modifying
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	// Set the viewport to be the entire window
	glViewport(0, 0, w, h);
	// Set the correct perspective.
	gluPerspective(45, ratio, 0.01, 1000);
	glMatrixMode(GL_MODELVIEW);
}

//point looked by camera
GLfloat x_reference = 0.0, y_reference = 0.0, z_reference = 0.0;

//point selected by the mouse
GLdouble mX, mY, mZ;

void display(void)
{
	GLfloat x_eye = 0.0, y_eye = 0.0, z_eye = 0.0;
	//clears the window

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//loads the identity matrix
	glLoadIdentity();

	//evaluate the camera location: as a result the current GL_MODELVIEW matrix will change

	x_eye = rho * sin(theta) * sin(phi) + x_reference;
	y_eye = rho * cos(theta) + y_reference;
	z_eye = rho * sin(theta) * cos(phi) + z_reference;

	//camera location (x_eye, y_eye, z_eye) and looked point (x_reference, y_reference, z_reference)
	gluLookAt(x_eye, y_eye, z_eye,
			  x_reference, y_reference, z_reference,
			  0.0, 1.0, 0.0);

	draw();

	//evaluates the point on the NURBS surface for the current value of [u, v]
	GLfloat x_s, y_s, z_s;
	eval_surf(&x_s, &y_s, &z_s, u, v);

	//draws the evaluated point on the NURBS surface
	set_selected_color();
	glPointSize(10.0);
	glBegin(GL_POINTS);
	glVertex3f(x_s, y_s, z_s);
	glEnd();

	//draws the point selected by the mouse
	glPointSize(10.0);
	glBegin(GL_POINTS);
	glVertex3f(mX, mY, mZ);
	glEnd();

	//axis
	set_lines_color();

	//x axis
	glBegin(GL_LINES);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(10.0, 0.0, 0.0);
	glEnd();
	renderBitmapString(3.0, 0.0, 0.0, GLUT_BITMAP_TIMES_ROMAN_24, "x");

	//y axis
	glBegin(GL_LINES);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, 10.0, 0.0);
	glEnd();
	renderBitmapString(0.0, 3.0, 0.0, GLUT_BITMAP_TIMES_ROMAN_24, "y");

	//z axis
	glBegin(GL_LINES);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, 0.0, 10.0);
	glEnd();
	renderBitmapString(0.0, 0.0, 3.0, GLUT_BITMAP_TIMES_ROMAN_24, "z");

	glFlush();
	glutSwapBuffers();

	glutSetWindow(curveWindow);
	displaySection();

	glutSetWindow(main_window_id);
}

void keyboardSpecial(int key, int x, int y)
{
	switch (key)
	{
	//right arrow
	case GLUT_KEY_RIGHT:
		phi += 0.1;
		break;

	//left arrow
	case GLUT_KEY_LEFT:
		phi += -0.1;
		break;

	//down arrow
	case GLUT_KEY_DOWN:
		theta += 0.1;
		break;

	//up arrow
	case GLUT_KEY_UP:
		theta += -0.1;
		break;
	}
}

#define zoomAmount 0.2

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{

	case 'a':
		x_reference += zoomAmount;
		break;

	case 'z':
		x_reference += -zoomAmount;
		break;

	case 's':
		y_reference += zoomAmount;
		break;

	case 'x':
		y_reference += -zoomAmount;
		break;

	case 'd':
		z_reference += zoomAmount;
		break;

	case 'c':
		z_reference += -zoomAmount;
		break;

	case 'q':
	case 'Q':
		releaseResource();
		exit(0);
		break;

	case '+':
		rho += -zoomAmount;
		break;

	case '-':
		rho += zoomAmount;
		break;

	case 'r':
		rho = 3.0;
		phi = 0.0;
		theta = PI_VALUE / 2.0;
		x_reference = 0.0;
		y_reference = 0.0;
		z_reference = 0.0;
		break;

	} //switch

	display();
}
///////////////////////////////////////////////////////////////////////
//////// end of functions and callback for the main window ////////////
///////////////////////////////////////////////////////////////////////

void releaseResource(void)
{
	gluDeleteNurbsRenderer(nurbs);
	gluDeleteNurbsRenderer(sectionNurbs);
	glDeleteTextures(1, &texName);
	delete[] ctrlpoints;
	delete[] skinning_ctrlpoints;
	delete[] sknots;
	delete[] tknots;
	delete[] skinning_tknots;
	delete[] multiple_tknots;
	delete[] multiple_ctrlpoints;
} //releaseResource

void knots_uniform(int select)
{
	int i;
	if (select == 1)
	{ //knots on S

		delete[] sknots;
		sknots = new GLfloat[S_NUMKNOTS];

		for (i = 0; i < S_NUMKNOTS; i++)
		{

			if (i < S_ORDER - 1)
				sknots[i] = 0.0;
			else
			{
				if (i > S_NUMKNOTS - S_ORDER)
					sknots[i] = 1.0;
				else if (S_NUMKNOTS - 2 * S_ORDER + 1 == 0)
					sknots[i] = 0.5;
				else
					sknots[i] = (float)((((float)(i - S_ORDER + 1)) /
										 ((float)(S_NUMKNOTS - 2 * S_ORDER + 1))));
			} //else
		}	  //for
	}		  //if

	if (select == 2)
	{ //knots on T

		delete[] tknots;
		tknots = new GLfloat[T_NUMKNOTS];

		for (i = 0; i < T_NUMKNOTS; i++)
		{
			if (i < T_ORDER - 1)
				tknots[i] = 0.0;
			else
			{
				if (i > T_NUMKNOTS - T_ORDER)
					tknots[i] = 1.0;
				else if (T_NUMKNOTS - 2 * T_ORDER + 1 == 0)
					tknots[i] = 0.5;
				else
					tknots[i] = (float)((((float)(i - T_ORDER + 1)) /
										 ((float)(T_NUMKNOTS - T_ORDER * 2 + 1))));
			} //else
		}	  //for
	}		  //if
} //knots_uniform

void find_barycentre(GLfloat *barycentre, int p_selected_column)
{
	int i = 0, j = 0;
	for (j = 0; j < 3; j++)
	{
		*(barycentre + j) = 0.0;
		for (i = 0; i < NCP_S; i++)
			*(barycentre + j) +=
				ctrlpoints[p_selected_column * 4 * NCP_S + i * 4 + j] /
				ctrlpoints[p_selected_column * 4 * NCP_S + i * 4 + 3]; //weight
		barycentre[j] /= NCP_S;
	}
}

void find_barycentre(GLfloat *barycentre)
{
	find_barycentre(barycentre, selected_column);
}

void init(void)
{
	int i = 0;
	char defaultFileBuffer[50];
	FILE *fp = fopen("defaultFile.config", "r");
	fscanf(fp, "%s", defaultFileBuffer);
	fclose(fp);
	strcpy(filename, defaultFileBuffer);
	old_transl_x[0] = 0;
	old_transl_y[0] = 0;
	old_transl_z[0] = 0;
	old_transl_zy[0] = 0;
	old_transl_zy[1] = 0;

	//set the background colours
	//glClearColor( 0.4, 0.5, 0.8, 0.0 ); //blue
	glClearColor(0.8, 0.8, 0.8, 0.0); //gray

	//lights
	GLfloat ambient[] =
		{0.8, 0.7, 0.7, 1.0};
	GLfloat specular[] =
		{1.0, 1.0, 4.0, 1.0};
	GLfloat diffuse[] =
		{0.4, 0.6, 1.0, 1.0};
	GLfloat position[] =
		{-0.5, 0.5, 0.5, 1.0};

	GLfloat spot_direction[] = {-1.0, -1.0, -1.0, 1.0};

	GLfloat spot_cutoff = 90.0;

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, position);
	glLightfv(GL_LIGHT0, GL_SPOT_CUTOFF, &spot_cutoff);
	glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, spot_direction);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_AUTO_NORMAL);
	glEnable(GL_NORMALIZE);
	glShadeModel(GL_SMOOTH);

	//sets the program exit callback
	atexit(releaseResource);

} //init

void copy_column()
{
	int i;
	for (i = 0; i < NCP_S * 4; i++)
	{
		if (i % 4 != 0)
			ctrlpoints[NCP_S * 4 * destination_column_copy + i] =
				ctrlpoints[NCP_S * 4 * selected_column + i];
	}

	if (display_skinning != 0)
		skinning();
}

//idle callback, used to sweep values of the [u, v] vector
void idle(void)
{
	if (v > (1.0 - v_inc))
		v_inc = -0.01;
	if (v <= (0.0 - v_inc))
		v_inc = 0.01;
	if (u >= 1.0)
		u_inc = -0.01;
	if (u <= (0.0 - u_inc))
	{
		u_inc = 0.01;
		v = v + v_inc;
	}
	u = u + u_inc;
	glutPostRedisplay();
}

void winToModelCoord(int x, int y, GLdouble *xModel, GLdouble *yModel, GLdouble *zModel)
{
	GLint viewport[4];
	GLdouble modelview[16];
	GLdouble projection[16];
	GLfloat winX, winY, winZ;

	glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	glGetIntegerv(GL_VIEWPORT, viewport);

	winX = (float)x;
	winY = (float)viewport[3] - (float)y;
	glReadPixels(x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);

	gluUnProject(winX, winY, winZ, modelview, projection, viewport, xModel, yModel, zModel);
}

GLfloat distance(GLfloat x1, GLfloat y1, GLfloat z1, GLfloat x2, GLfloat y2, GLfloat z2)
{
	return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2) + (z1 - z2) * (z1 - z2));
}

//call back for mouse button events
void mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON)
	{
		winToModelCoord(x, y, &mX, &mY, &mZ);
	}
	if (button == GLUT_RIGHT_BUTTON)
	{ //selects a control point
		GLdouble local_mX, local_mY, local_mZ;
		winToModelCoord(x, y, &local_mX, &local_mY, &local_mZ);
		GLfloat cur_x, cur_y, cur_z, cur_w;
		GLfloat threshold = 0.1;
		for (int i = 0; i < NCP_T; i++)
		{
			for (int j = 0; j < NCP_S; j++)
			{
				cur_x = ctrlpoints[i * NCP_S * 4 + j * 4];
				cur_y = ctrlpoints[i * NCP_S * 4 + j * 4 + 1];
				cur_z = ctrlpoints[i * NCP_S * 4 + j * 4 + 2];
				cur_w = ctrlpoints[i * NCP_S * 4 + j * 4 + 3];
				if (distance(cur_x / cur_w, cur_y / cur_w, cur_z / cur_w, local_mX, local_mY, local_mZ) < threshold)
				{
					indexSelectedCtrlPoint = i * NCP_S * 4 + j * 4;
					weightSelectedCtrlPoint = ctrlpoints[indexSelectedCtrlPoint + 3];
					glui->sync_live();
					return;
				}
			}
		}
	}
}

//call back for mouse movement events
void mouseMotion(int x, int y)
{
	//winToModelCoord(x, y, &mX, &mY, &mZ);
}

GLUI_Rollout *rollout_knot_s, *rollout_knot_t;
GLUI_Panel *cmd_panel, *panel_cols, *panel, *multiplicity_panel;
GLUI_RadioGroup *radioGroupCols;

/// to be called after an update to one of the following variable: sknots, tknots, NCT_T, T_ORDER
void gluiSurfaceParametersInit()
{
	char buffer2[10] = "";
	int i = 0;

	if (rollout_knot_s != NULL)
		rollout_knot_s->unlink();
	if (rollout_knot_t != NULL)
		rollout_knot_t->unlink();
	if (radioGroupCols != NULL)
		radioGroupCols->unlink();
	if (spinner_multiplicity != NULL)
		spinner_multiplicity->unlink();

	rollout_knot_s = glui->add_rollout_to_panel(cmd_panel, "knots in s", false);
	rollout_knot_t = glui->add_rollout_to_panel(cmd_panel, "knots in t", false);
	radioGroupCols = glui->add_radiogroup_to_panel(panel_cols, &selected_column, -1);

	for (i = 0; i < S_NUMKNOTS; i++)
	{
		sprintf(buffer2, "%d", i);
		glui->add_spinner_to_panel(rollout_knot_s, buffer2, GLUI_SPINNER_FLOAT, sknots + i);
	}
	for (i = 0; i < T_NUMKNOTS; i++)
	{
		sprintf(buffer2, "%d", i);
		glui->add_spinner_to_panel(rollout_knot_t, buffer2, GLUI_SPINNER_FLOAT, tknots + i);
	}

	for (i = 0; i < NCP_T; i++)
	{
		sprintf(buffer2, "%d", i);
		glui->add_radiobutton_to_group(radioGroupCols, buffer2);
	}

	spinner_multiplicity = glui->add_spinner_to_panel(multiplicity_panel, "multipl.", GLUI_SPINNER_INT, &multiplicity);
	spinner_multiplicity->set_speed(0.002);
	spinner_multiplicity->set_int_limits(1, T_ORDER - 1);
	spinner_multiplicity->callback = (GLUI_Update_CB)initMultipleCtrlPoints;
	if ((spinner_multiplicity != NULL) && (spinner_destination != NULL))
	{
		spinner_multiplicity->set_int_limits(1, T_ORDER - 1);
		spinner_destination->set_int_limits(0, NCP_T - 1);
	} //if

	newDegreeS = S_ORDER - 1;
	newDegreeT = T_ORDER - 1;
	glui->sync_live();
}

void do_weight_adjust(void)
{
	GLfloat old_weight = ctrlpoints[indexSelectedCtrlPoint + 3];
	ctrlpoints[indexSelectedCtrlPoint] = ctrlpoints[indexSelectedCtrlPoint] / old_weight * weightSelectedCtrlPoint;
	ctrlpoints[indexSelectedCtrlPoint + 1] = ctrlpoints[indexSelectedCtrlPoint + 1] / old_weight * weightSelectedCtrlPoint;
	ctrlpoints[indexSelectedCtrlPoint + 2] = ctrlpoints[indexSelectedCtrlPoint + 2] / old_weight * weightSelectedCtrlPoint;
	ctrlpoints[indexSelectedCtrlPoint + 3] = weightSelectedCtrlPoint;
}

int main(int argc, char **argv)
{
	guiInit(argc, argv);
	glutMainLoop();
	return 0;
}
