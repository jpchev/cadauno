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

#ifndef EXTERN
#define EXTERN

#include "include/glui.h"

void knots_uniform(int select);
extern int display_skinning;
extern GLfloat nurbsDisplayMode;
extern int nurbsDisplayModeInt;
extern int showCtrlPtsPlanes;
extern GLfloat weightSelectedCtrlPoint;
extern GLfloat barycentre[3];
void find_barycentre(GLfloat *, int);
extern int selected_column;
extern int destination_column_copy;

extern int Window_Width;
extern int Window_Height;

extern int Window_Width_section;
extern int Window_Height_section;

void aspectRatio(int w, int h);
void mouse(int button, int state, int x, int y);
void mouseMotion(int x, int y);
void idle();
void keyboardSpecial(int key, int x, int y);
void keyboard(unsigned char key, int x, int y);
void init();

/////////////////////////////////////////////////////////////////////////////
////////////////////// variables for the background texture /////////////////
/////////////////////////////////////////////////////////////////////////////
extern GLuint texName;
/////////////////////////////////////////////////////////////////////////////
///////////////////////// end of variables for the background ///////////////
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/////////////////////////// the GLUI interface //////////////////////////////
/////////////////////////////////////////////////////////////////////////////
extern GLUI *glui;
extern char filename[sizeof(GLUI_String)];
extern char background[sizeof(GLUI_String)];
extern GLfloat transl_x[1], transl_y[1], transl_z[1], transl_zy[2];
extern GLfloat old_transl_x[1], old_transl_y[1], old_transl_z[1], old_transl_zy[2];
extern GLfloat rotation_data[16];
extern GLUI_Rotation *rotation;
extern GLfloat z_rotation;
extern GLUI_Spinner *z_rot_spinner, *spinner_multiplicity, *spinner_destination;
void gluiSurfaceParametersInit();
extern GLUI_Panel *cmd_panel;
extern GLUI_Panel *panel;
extern GLUI_Panel *panel_cols;
extern GLUI_Panel *multiplicity_panel;
extern GLUI_Translation *x_translation, *y_translation, *z_translation, *zy_translation;
extern GLUI_Rotation *rotation;
extern GLUI_Spinner *z_rot_spinner, *spinner_multiplicity, *spinner_destination;
/////////////////////////////////////////////////////////////////////////////
////////////////////// end of the GLUI interface ////////////////////////////
/////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
//////////////////// ctrlpoints ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////
//number of control points
extern int NCP_S;
extern int NCP_T;

//curves order(equals degree + 1)
extern int S_ORDER;

/*order in the t direction(equals degree + 1)
and multiplicity + 1 of control points in the t direction
(boundaries not included) which enables the close NURBS
to go through the section curves*/
extern int T_ORDER;

//useful to modify orders
extern int newDegreeS, newDegreeT;

//knots number, it always must equals NCP_S + S_ORDER and NCP_T + T_ORDER
extern int S_NUMKNOTS;
extern int T_NUMKNOTS;

//control points of the patch
//given by columns, NCP_S are points on the columns, NCP_T those on "rows"
extern GLfloat *skinning_ctrlpoints;

//control points are stored here
extern GLfloat *ctrlpoints;

//the number of knots sknots must equals S_ORDER + NCP_S
extern GLfloat *sknots;

//the number of knots tknots must equals T_ORDER + NCP_T
extern GLfloat *tknots;

extern GLfloat *skinning_tknots;

extern int multiplicity;
extern GLfloat *multiple_tknots, *multiple_ctrlpoints;

int findspan(int n, int p, float u, float *U);
///////////////////////////////////////////////////////////////////////
//////////////////////// end ctrlpoints ///////////////////////////////
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
////////////////////////// selected section curve /////////////////////
///////////////////////////////////////////////////////////////////////
extern GLint selected_point;

extern GLUnurbs *sectionNurbs;

extern GLfloat scale_section;

extern int section_ctrlpoint_subMenu;

void displaySection(void);

void drawSection();

extern int curveWindow, main_window_id;

void mouseMotionSection(int x, int y);

void mousePassiveMotionSection(int x, int y);

void mouseSection(int button, int state, int x, int y);

void aspectRatioSection(int w, int h);

void keyboardSectionSpecial(int key, int x, int y);

void keyboardSection(unsigned char key, int x, int y);
///////////////////////////////////////////////////////////////////////
///////////////////////// end selected section curve //////////////////
///////////////////////////////////////////////////////////////////////

void display();
void skinning();

typedef enum surfType
{
    Revolving_surfType,
    Extruded_surfType
} surfType;
extern surfType newSurfType;

//gui callbacks
void load_points(int id);
void save_points(int id);
void load_background();
void copy_column();
void do_scale_up();
void do_scale_down();
void do_transl_zy();
void do_transl_z();
void do_transl_x();
void do_transl_y();
void do_weight_adjust();
void do_z_rotation();
void do_rotation();

void nurbsError(GLenum errorCode);

#endif