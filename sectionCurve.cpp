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

#include <math.h>
#include "extern.h"
#include "include/FreeImage.h"
#include "entities.h"

#ifdef _WIN32
#define STDCALL __stdcall
#else
#define STDCALL
#endif

GLint selected_point = 0;

GLUnurbs *sectionNurbs = NULL;

GLfloat scale_section = 1.0;

GLint imageWidth = 0;
GLint imageHeight = 0;
GLuint texName = 0;

/** Generic image loader
	@param lpszPathName Pointer to the full file name
	@param flag Optional load flag constant
	@return Returns the loaded dib if successful, returns NULL otherwise
*/
FIBITMAP *GenericLoader(const char *lpszPathName, int flag)
{
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;

	// check the file signature and deduce its format
	// (the second argument is currently not used by FreeImage)
	fif = FreeImage_GetFileType(lpszPathName, 0);
	if (fif == FIF_UNKNOWN)
	{
		// no signature ?
		// try to guess the file format from the file extension
		fif = FreeImage_GetFIFFromFilename(lpszPathName);
	}
	// check that the plugin has reading capabilities ...
	if ((fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif))
	{
		// ok, let's load the file
		FIBITMAP *dib = FreeImage_Load(fif, lpszPathName, 0);
		// unless a bad file format, we are done !

		// Now, there is no guarantee that the image file
		// loaded will be GL_RGB, so we force FreeImage to
		// convert the image to GL_RGB.
		FIBITMAP *temp = dib;
		dib = FreeImage_ConvertTo32Bits(dib);
		FreeImage_Unload(temp);

		return dib;
	}
	return NULL;
}

void load_background()
{

	//save the current window id
	int win = glutGetWindow();

	//set the section window as the active window
	glutSetWindow(curveWindow);

	FIBITMAP *image = GenericLoader(background, 0);
	if (image == NULL)
	{
		strcpy(background, "");
		return;
	}

	glGenTextures(1, &texName);

	glBindTexture(GL_TEXTURE_2D, texName);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	imageWidth = FreeImage_GetWidth(image);
	imageHeight = FreeImage_GetHeight(image);

	unsigned char *textura = new unsigned char[4 * imageWidth * imageHeight];
	unsigned char *pixeles = FreeImage_GetBits(image);

	for (int j = 0; j < imageWidth * imageHeight; j++)
	{
		textura[j * 4 + 0] = pixeles[j * 4 + 0];
		textura[j * 4 + 1] = pixeles[j * 4 + 1];
		textura[j * 4 + 2] = pixeles[j * 4 + 2];
		textura[j * 4 + 3] = pixeles[j * 4 + 3];
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
				 imageWidth, imageHeight,
				 0, GL_RGBA,
				 GL_UNSIGNED_BYTE,
				 (GLvoid *)textura);

	FreeImage_Unload(image);
	delete[] textura;

	//restore the window in use when calling this function
	glutSetWindow(win);
}

///////////////////////////////////////////////////////////////////////
/////////////////////// section curve drawing /////////////////////////
///////////////////////////////////////////////////////////////////////
void drawSection()
{
	int i = 0;

	GLfloat *selection_ctrlpoints = ctrlpoints + selected_column * 4 * NCP_S;

	glColor3f(1.0, 1.0, 1.0);

	if (sectionNurbs == NULL)
	{
		sectionNurbs = gluNewNurbsRenderer();
		gluNurbsCallback(sectionNurbs, GLU_ERROR, (void(STDCALL *)(void))nurbsError);
	}

	glScalef(
		scale_section,
		scale_section,
		scale_section);

	//renders the texture, if present
	glBindTexture(GL_TEXTURE_2D, texName);

	GLfloat textureWidth = 1.0,
			ratio = (GLfloat)((GLfloat)imageHeight / (GLfloat)imageWidth);

	//point to be calculated according to the normal vector, to do
	point3d bottomLeftTexture(0, -ratio * textureWidth / 2.0, -textureWidth / 2.0),
		bottomRightTexture(0, -ratio * textureWidth / 2.0, textureWidth / 2.0),
		topRightTexture(0.0, ratio * textureWidth / 2.0, textureWidth / 2.0),
		topLeftTexture(0.0, ratio * textureWidth / 2.0, -textureWidth / 2.0);

	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(bottomLeftTexture.x, bottomLeftTexture.y, bottomLeftTexture.z);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(bottomRightTexture.x, bottomRightTexture.y, bottomRightTexture.z);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(topRightTexture.x, topRightTexture.y, topRightTexture.z);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(topLeftTexture.x, topLeftTexture.y, topLeftTexture.z);
	glEnd();
	glDisable(GL_TEXTURE_2D);

	/*
	section curve and its control points
	*/
	gluBeginCurve(sectionNurbs);
	gluNurbsCurve(
		sectionNurbs,
		S_NUMKNOTS, sknots,
		4, selection_ctrlpoints,
		S_ORDER,
		GL_MAP1_VERTEX_4);
	gluEndCurve(sectionNurbs);

	glColor3f(1.0, 0.0, 1.0);
	glPointSize(5.0);

	glBegin(GL_POINTS);
	for (i = 0; i < NCP_S; i++)
	{

		if (i == selected_point)
			glColor3f(0.8, 0.2, 0.3);

		glVertex4fv(selection_ctrlpoints + i * 4);

		if (i == selected_point)
			glColor3f(1.0, 0.0, 1.0);
	}

	glColor3f(0.3, 0.7, 0.2);
	glVertex4fv(barycentre);
	glEnd();

	//glColor3f(1.0, 1.0, 0.0);
	//glBegin(GL_LINE_STRIP);
	//for(i=0; i<NCP_S; i++)
	//	glVertex4fv(selection_ctrlpoints + i*4);
	//glEnd();

} //drawSection
///////////////////////////////////////////////////////////////////////
////////////////////////// end of section curve ///////////////////////
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
/////////////////// window callback for the section curve /////////////
///////////////////////////////////////////////////////////////////////
void aspectRatioSection(int w, int h)
{
	Window_Width_section = w;
	Window_Height_section = h;
	//Set the transformations
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, 0.0, 100.0);
	glMatrixMode(GL_MODELVIEW);
	glViewport(0, 0, w, h);
}

vector3d normalVect(-5, 0, 0), offsetVec(0, 0, 0);

/**
	returns the reference point, depends on the normalVect
*/
point3d getSectionReference()
{
	point3d reference(barycentre[0], barycentre[1], barycentre[2]);
	reference = reference + offsetVec;
	return reference;
}

/**
	returns the reference point, depends on the offsetVec
*/
point3d getSectionEye()
{
	point3d eye = getSectionReference() + normalVect;
	return eye;
}

void displaySection(void)
{
	//clears the window
	glClear(GL_COLOR_BUFFER_BIT);

	//load the identity matrix
	glLoadIdentity();

	point3d eye = getSectionEye();
	point3d reference = getSectionReference();

	gluLookAt(
		eye.x,
		eye.y,
		eye.z,

		reference.x,
		reference.y,
		reference.z,

		0.0, 1.0, 0.0);

	//draws the section
	drawSection();

	//draws axis

	//x axis
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_LINES);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(10.0, 0.0, 0.0);
	glEnd();

	//y axis
	glColor3f(0.0, 1.0, 0.0);
	glBegin(GL_LINES);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, 10.0, 0.0);
	glEnd();

	//z axis
	glColor3f(0.0, 0.0, 10.0);
	glBegin(GL_LINES);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, 0.0, 2.0);
	glEnd();

	glFlush();
	glutSwapBuffers();
}

void winToModelCoordSection(int x, int y, GLdouble *xModel, GLdouble *yModel, GLdouble *zModel)
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

	GLdouble pt_winX, pt_winY, pt_winZ; //window coordinate of the selected control point in the selected column
	gluProject(ctrlpoints[selected_column * 4 * NCP_S + selected_point * 4 + 0] /
				   ctrlpoints[selected_column * 4 * NCP_S + selected_point * 4 + 3],

			   ctrlpoints[selected_column * 4 * NCP_S + selected_point * 4 + 1] /
				   ctrlpoints[selected_column * 4 * NCP_S + selected_point * 4 + 3],

			   ctrlpoints[selected_column * 4 * NCP_S + selected_point * 4 + 2] /
				   ctrlpoints[selected_column * 4 * NCP_S + selected_point * 4 + 3],

			   modelview, projection, viewport,

			   &pt_winX,
			   &pt_winY,
			   &pt_winZ);

	gluUnProject(winX, winY, pt_winZ, modelview, projection, viewport, xModel, yModel, zModel);
}

int mouseButton = -1;
int mouseX = 0, mouseY = 0;

void mousePassiveMotionSection(int x, int y)
{
	mouseX = x;
	mouseY = y;
}

//call back for mouse movement events
void mouseMotionSection(int x, int y)
{

	if (mouseButton != GLUT_LEFT_BUTTON)
		return;

	GLdouble xModel, yModel, zModel;

	/* Translate back to our coordinate system */
	winToModelCoordSection(x, y, &xModel, &yModel, &zModel);

	//crtlpoints update
	ctrlpoints[selected_column * 4 * NCP_S +
			   selected_point * 4 + 0] = xModel *
										 //point weight
										 ctrlpoints[selected_column * 4 * NCP_S + selected_point * 4 + 3];

	ctrlpoints[selected_column * 4 * NCP_S +
			   selected_point * 4 + 1] = yModel *
										 //point weight
										 ctrlpoints[selected_column * 4 * NCP_S + selected_point * 4 + 3];

	ctrlpoints[selected_column * 4 * NCP_S +
			   selected_point * 4 + 2] = zModel *
										 //point weight
										 ctrlpoints[selected_column * 4 * NCP_S + selected_point * 4 + 3];

	displaySection();
	glutSetWindow(main_window_id);

	if (display_skinning != 0)
		skinning();

	display();
	glutSetWindow(curveWindow);
}

//call back for mouse button events
void mouseSection(int button, int state, int x, int y)
{
	GLdouble xModel, yModel, zModel;

	mouseButton = button;

	/* We are only interested in left clicks */
	if (button != GLUT_LEFT_BUTTON)
		return;

	/* Translate back to our coordinate system */
	winToModelCoordSection(x, y, &xModel, &yModel, &zModel);

	if (state == GLUT_DOWN)
	{
		float d, min_d = -1.0;

		for (int i = 0; i < NCP_S; i++)
		{
			d = point3d(
					ctrlpoints[selected_column * 4 * NCP_S + i * 4 + 0] /
						ctrlpoints[selected_column * 4 * NCP_S + i * 4 + 3], //point weight

					ctrlpoints[selected_column * 4 * NCP_S + i * 4 + 1] /
						ctrlpoints[selected_column * 4 * NCP_S + i * 4 + 3], //point weight

					ctrlpoints[selected_column * 4 * NCP_S + i * 4 + 2] /
						ctrlpoints[selected_column * 4 * NCP_S + i * 4 + 3] //point weight
					)
					.distance(point3d(xModel, yModel, zModel));

			if (min_d == -1.0)
			{
				selected_point = i;
				min_d = d;
			}

			if (d < min_d)
			{
				selected_point = i;
				min_d = d;
			}
		}
		//return;
	}

	//crtlpoints update
	ctrlpoints[selected_column * 4 * NCP_S +
			   selected_point * 4 + 0] = xModel *
										 //point weight
										 ctrlpoints[selected_column * 4 * NCP_S + selected_point * 4 + 3];

	ctrlpoints[selected_column * 4 * NCP_S +
			   selected_point * 4 + 1] = yModel *
										 //point weight
										 ctrlpoints[selected_column * 4 * NCP_S + selected_point * 4 + 3];
	ctrlpoints[selected_column * 4 * NCP_S +
			   selected_point * 4 + 2] = zModel *
										 //point weight
										 ctrlpoints[selected_column * 4 * NCP_S + selected_point * 4 + 3];

	displaySection();
	glutSetWindow(main_window_id);

	if (display_skinning != 0)
		skinning();

	display();
	glutSetWindow(curveWindow);
}

void keyboardSectionSpecial(int key, int x, int y)
{
	point3d eye = getSectionEye();
	point3d reference = getSectionReference();

	vector3d horizontalRightIncrement = (vector3d(0, 1, 0).crossProd(eye - reference)).normalize() * 0.1;

	switch (key)
	{
	case GLUT_KEY_LEFT:
		offsetVec = offsetVec - horizontalRightIncrement;
		break;
	case GLUT_KEY_RIGHT:
		offsetVec = offsetVec + horizontalRightIncrement;
		break;
	case GLUT_KEY_UP:
		offsetVec.y += 0.1;
		break;
	case GLUT_KEY_DOWN:
		offsetVec.y -= 0.1;
		break;
	} //switch

	displaySection();
	glutSetWindow(main_window_id);
	display();
	glutSetWindow(curveWindow);
}

void keyboardSection(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'q':
	case 'Q':
		exit(0);
		break;

	case 'n':
		if (selected_point < NCP_S - 1)
			selected_point++;
		else
			selected_point = 0;
		break;

	case '-':
		scale_section -= 0.25;
		break;

	case '+':
		scale_section += 0.25;
		break;
	} //switch

	displaySection();
	glutSetWindow(main_window_id);
	display();
	glutSetWindow(curveWindow);
}
///////////////////////////////////////////////////////////////////////
//////////// end of window callback for the section curve /////////////
///////////////////////////////////////////////////////////////////////

void addCtrlPoint()
{
	GLdouble xModel, yModel, zModel;
	winToModelCoordSection(mouseX, mouseY, &xModel, &yModel, &zModel);

	GLfloat *ctrlpointsNew = new GLfloat[(NCP_S + 1) * NCP_T * 4];

	int pointBeforeNewIndex = -1;
	float d1, d2, min_d = -1.0;
	for (int i = 1; i < NCP_S; i++)
	{
		d1 = point3d(
				 ctrlpoints[selected_column * 4 * NCP_S + i * 4 + 0] /
					 ctrlpoints[selected_column * 4 * NCP_S + i * 4 + 3], //point weight

				 ctrlpoints[selected_column * 4 * NCP_S + i * 4 + 1] /
					 ctrlpoints[selected_column * 4 * NCP_S + i * 4 + 3], //point weight

				 ctrlpoints[selected_column * 4 * NCP_S + i * 4 + 2] /
					 ctrlpoints[selected_column * 4 * NCP_S + i * 4 + 3] //point weight
				 )
				 .distance(point3d(xModel, yModel, zModel));

		d2 = point3d(
				 ctrlpoints[selected_column * 4 * NCP_S + (i - 1) * 4 + 0] /
					 ctrlpoints[selected_column * 4 * NCP_S + (i - 1) * 4 + 3], //point weight

				 ctrlpoints[selected_column * 4 * NCP_S + (i - 1) * 4 + 1] /
					 ctrlpoints[selected_column * 4 * NCP_S + (i - 1) * 4 + 3], //point weight

				 ctrlpoints[selected_column * 4 * NCP_S + (i - 1) * 4 + 2] /
					 ctrlpoints[selected_column * 4 * NCP_S + (i - 1) * 4 + 3] //point weight
				 )
				 .distance(point3d(xModel, yModel, zModel));

		if (min_d == -1.0)
		{
			pointBeforeNewIndex = i - 1;
			min_d = d1 + d2;
		}

		if (d1 + d2 < min_d)
		{
			pointBeforeNewIndex = i - 1;
			min_d = d1 + d2;
		}
	}

	find_barycentre(barycentre, selected_column);

	//ctrl points update
	for (int i = 0; i < NCP_T; i++)
	{ //column number
		for (int j = 0; j < NCP_S + 1; j++)
		{

			if (j <= pointBeforeNewIndex)
			{
				ctrlpointsNew[i * (NCP_S + 1) * 4 + j * 4 + 0] = ctrlpoints[i * NCP_S * 4 + j * 4 + 0];
				ctrlpointsNew[i * (NCP_S + 1) * 4 + j * 4 + 1] = ctrlpoints[i * NCP_S * 4 + j * 4 + 1];
				ctrlpointsNew[i * (NCP_S + 1) * 4 + j * 4 + 2] = ctrlpoints[i * NCP_S * 4 + j * 4 + 2];
				ctrlpointsNew[i * (NCP_S + 1) * 4 + j * 4 + 3] = ctrlpoints[i * NCP_S * 4 + j * 4 + 3];
			}

			GLfloat current_column_bary[3];
			find_barycentre(current_column_bary, i);
			point3d selectedColumnBarycentrePoint(barycentre[0], barycentre[1], barycentre[2]);
			point3d currentColumnBarycentrePoint(current_column_bary[0], current_column_bary[1], current_column_bary[2]);
			vector3d barycentresDistance = currentColumnBarycentrePoint - selectedColumnBarycentrePoint;
			point3d newPoint(xModel, yModel, zModel);
			newPoint = newPoint + barycentresDistance;

			if (j == pointBeforeNewIndex + 1)
			{
				ctrlpointsNew[i * (NCP_S + 1) * 4 + j * 4 + 0] = newPoint.x;
				ctrlpointsNew[i * (NCP_S + 1) * 4 + j * 4 + 1] = newPoint.y;
				ctrlpointsNew[i * (NCP_S + 1) * 4 + j * 4 + 2] = newPoint.z;
				ctrlpointsNew[i * (NCP_S + 1) * 4 + j * 4 + 3] = 1.0;
			}

			if (j > pointBeforeNewIndex + 1)
			{
				ctrlpointsNew[i * (NCP_S + 1) * 4 + j * 4 + 0] = ctrlpoints[i * NCP_S * 4 + (j - 1) * 4 + 0];
				ctrlpointsNew[i * (NCP_S + 1) * 4 + j * 4 + 1] = ctrlpoints[i * NCP_S * 4 + (j - 1) * 4 + 1];
				ctrlpointsNew[i * (NCP_S + 1) * 4 + j * 4 + 2] = ctrlpoints[i * NCP_S * 4 + (j - 1) * 4 + 2];
				ctrlpointsNew[i * (NCP_S + 1) * 4 + j * 4 + 3] = ctrlpoints[i * NCP_S * 4 + (j - 1) * 4 + 3];
			}
		}
	}

	//modify knots
	point3d newPoint(xModel, yModel, zModel);
	point3d pointBeforeNew(
		ctrlpoints[selected_column * 4 * NCP_S + pointBeforeNewIndex * 4 + 0] /
			ctrlpoints[selected_column * 4 * NCP_S + pointBeforeNewIndex * 4 + 3],

		ctrlpoints[selected_column * 4 * NCP_S + pointBeforeNewIndex * 4 + 1] /
			ctrlpoints[selected_column * 4 * NCP_S + pointBeforeNewIndex * 4 + 3],

		ctrlpoints[selected_column * 4 * NCP_S + pointBeforeNewIndex * 4 + 2] /
			ctrlpoints[selected_column * 4 * NCP_S + pointBeforeNewIndex * 4 + 3]);
	int pointAfterNewIndex = pointBeforeNewIndex < NCP_S - 1 ? pointBeforeNewIndex + 1 : 0;
	point3d pointAfterNew(
		ctrlpoints[selected_column * 4 * NCP_S + pointAfterNewIndex * 4 + 0] /
			ctrlpoints[selected_column * 4 * NCP_S + pointAfterNewIndex * 4 + 3],

		ctrlpoints[selected_column * 4 * NCP_S + pointAfterNewIndex * 4 + 1] /
			ctrlpoints[selected_column * 4 * NCP_S + pointAfterNewIndex * 4 + 3],

		ctrlpoints[selected_column * 4 * NCP_S + pointAfterNewIndex * 4 + 2] /
			ctrlpoints[selected_column * 4 * NCP_S + pointAfterNewIndex * 4 + 3]);

	GLfloat wi_1 = ctrlpoints[selected_column * 4 * NCP_S + pointBeforeNewIndex * 4 + 3];
	GLfloat wi = ctrlpoints[selected_column * 4 * NCP_S + pointAfterNewIndex * 4 + 3];

	GLfloat s = wi_1 * newPoint.distance(pointBeforeNew) /
				(wi_1 * newPoint.distance(pointBeforeNew) + wi * pointAfterNew.distance(newPoint));

	GLfloat new_knot_value = sknots[pointAfterNewIndex] +
							 s * (sknots[pointAfterNewIndex + S_ORDER - 1] - sknots[pointAfterNewIndex]);

	int k = findspan(NCP_S, S_ORDER - 1, new_knot_value, sknots) - 1;

	S_NUMKNOTS++;

	GLfloat *new_sknots = new GLfloat[S_NUMKNOTS];
	for (int i = 0; i < S_NUMKNOTS; i++)
	{
		if (i <= k)
			new_sknots[i] = sknots[i];
		if (i == k + 1)
			new_sknots[i] = new_knot_value;
		if (i > k + 1)
			new_sknots[i] = sknots[i - 1];
	}
	delete[] sknots;
	sknots = new_sknots;

	NCP_S++;

	delete[] ctrlpoints;
	ctrlpoints = ctrlpointsNew;

	gluiSurfaceParametersInit();
	glui->sync_live();
}

void removeCtrlPoint()
{
	GLdouble xModel, yModel, zModel;
	winToModelCoordSection(mouseX, mouseY, &xModel, &yModel, &zModel);

	GLfloat *ctrlpointsNew = new GLfloat[(NCP_S - 1) * NCP_T * 4];

	float d, min_d = 1000.0;
	int selected_point = -1;
	for (int i = 0; i < NCP_S; i++)
	{
		d = point3d(
				ctrlpoints[selected_column * 4 * NCP_S + i * 4 + 0] /
					ctrlpoints[selected_column * 4 * NCP_S + i * 4 + 3], //point weight

				ctrlpoints[selected_column * 4 * NCP_S + i * 4 + 1] /
					ctrlpoints[selected_column * 4 * NCP_S + i * 4 + 3], //point weight

				ctrlpoints[selected_column * 4 * NCP_S + i * 4 + 2] /
					ctrlpoints[selected_column * 4 * NCP_S + i * 4 + 3] //point weight
				)
				.distance(point3d(xModel, yModel, zModel));

		if (d < min_d)
		{
			selected_point = i;
			min_d = d;
		}
	}

	if (selected_point < 0)
		return; //if point not found

	NCP_S--;
	S_NUMKNOTS--;

	//ctrl points update
	for (int i = 0; i < NCP_T; i++)
	{
		for (int j = 0; j < NCP_S; j++)
		{

			if (j < selected_point)
			{
				ctrlpointsNew[i * (NCP_S)*4 + j * 4 + 0] = ctrlpoints[i * (NCP_S + 1) * 4 + j * 4 + 0];
				ctrlpointsNew[i * (NCP_S)*4 + j * 4 + 1] = ctrlpoints[i * (NCP_S + 1) * 4 + j * 4 + 1];
				ctrlpointsNew[i * (NCP_S)*4 + j * 4 + 2] = ctrlpoints[i * (NCP_S + 1) * 4 + j * 4 + 2];
				ctrlpointsNew[i * (NCP_S)*4 + j * 4 + 3] = ctrlpoints[i * (NCP_S + 1) * 4 + j * 4 + 3];
			}

			if (j >= selected_point)
			{
				ctrlpointsNew[i * (NCP_S)*4 + j * 4 + 0] = ctrlpoints[i * (NCP_S + 1) * 4 + (j + 1) * 4 + 0];
				ctrlpointsNew[i * (NCP_S)*4 + j * 4 + 1] = ctrlpoints[i * (NCP_S + 1) * 4 + (j + 1) * 4 + 1];
				ctrlpointsNew[i * (NCP_S)*4 + j * 4 + 2] = ctrlpoints[i * (NCP_S + 1) * 4 + (j + 1) * 4 + 2];
				ctrlpointsNew[i * (NCP_S)*4 + j * 4 + 3] = ctrlpoints[i * (NCP_S + 1) * 4 + (j + 1) * 4 + 3];
			}
		}
	}

	delete[] ctrlpoints;
	ctrlpoints = ctrlpointsNew;

	//modify knots
	knots_uniform(1);
	gluiSurfaceParametersInit();
	glui->sync_live();
}
