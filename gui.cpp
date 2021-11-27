#include "extern.h"
#include "serializator.h"
#include "entities.h"
#include "surfaces.h"

surfType newSurfType;

void newRevolvedSurf()
{
	NCP_T = 9;	 //in the T direction 9 ctrl points are used
	T_ORDER = 3; //in the T direction degree 2 is used

	//input
	point3d S(0, 0, 0);
	vector3d T(0, 1, 0);
	float theta = 2.0 * PI;
	point3d *Pj = (point3d *)malloc(NCP_S * sizeof(point3d));
	float *wj = (float *)malloc(NCP_S * sizeof(GLfloat));
	for (int i = 0; i < NCP_S; i++)
	{
		Pj[i] = point3d(ctrlpoints[selected_column * 4 * NCP_S + i * 4 + 0],
						ctrlpoints[selected_column * 4 * NCP_S + i * 4 + 1],
						ctrlpoints[selected_column * 4 * NCP_S + i * 4 + 2]);
		Pj[i] = Pj[i] * (1.0 / ctrlpoints[selected_column * 4 * NCP_S + i * 4 + 3]); //weight

		wj[i] = ctrlpoints[selected_column * 4 * NCP_S + i * 4 + 3]; //weight
	}
	//input end

	int m = NCP_S - 1;

	T_NUMKNOTS = T_ORDER + NCP_T;

	//output
	int n;
	float *U = (float *)malloc(T_NUMKNOTS * sizeof(float));
	point3d *Pij = (point3d *)malloc(NCP_S * 9 * sizeof(point3d));
	float *wij = (float *)malloc(NCP_S * 9 * sizeof(GLfloat));

	makeRevolvedSurf(
		//input
		S,	   //point
		T,	   //unit length vector
		theta, //angle in radians
		m, Pj, wj,

		//output
		&n, U, Pij, wij);

	//copying computed ctrl points
	if (ctrlpoints != NULL)
	{
		delete[] ctrlpoints;
		delete[] multiple_ctrlpoints;
		multiple_ctrlpoints = NULL;
	}

	ctrlpoints = new GLfloat[NCP_S * NCP_T * 4];

	for (int i = 0; i < NCP_S * 9; i++)
	{
		ctrlpoints[i * 4 + 0] = Pij[i].x * wij[i];
		ctrlpoints[i * 4 + 1] = Pij[i].y * wij[i];
		ctrlpoints[i * 4 + 2] = Pij[i].z * wij[i];
		ctrlpoints[i * 4 + 3] = wij[i];
	}

	//copying computed knots in the T direction
	delete[] tknots;
	tknots = new GLfloat[T_NUMKNOTS];
	for (int i = 0; i < T_NUMKNOTS; i++)
	{
		tknots[i] = U[i];
	}

	free(Pj);
	free(wj);
	free(Pij);
	free(wij);
	free(U);
}

void createNewSurf()
{

	//revolving surface about an axis
	switch (newSurfType)
	{
	case Revolving_surfType:
		newRevolvedSurf();
		break;
	case Extruded_surfType:
		break;
	}
	gluiSurfaceParametersInit();
}

void addCtrlPoint();
void removeCtrlPoint();

void menu(int op)
{
	switch (op)
	{
	case 1:
		addCtrlPoint();
		break;
	case 2:
		removeCtrlPoint();
		break;
	}
}

int section_ctrlpoint_subMenu;

int newDegreeS, newDegreeT;

void changeDegreeS()
{
	S_ORDER = newDegreeS + 1;
	S_NUMKNOTS = S_ORDER + NCP_S;
	knots_uniform(1);
	gluiSurfaceParametersInit();
}

void changeDegreeT()
{
	T_ORDER = newDegreeT + 1;
	T_NUMKNOTS = T_ORDER + NCP_T;
	knots_uniform(2);
	gluiSurfaceParametersInit();
}

void setFillNurbs()
{
	if (nurbsDisplayModeInt == 1)
		nurbsDisplayMode = GLU_FILL;
	else
		nurbsDisplayMode = GLU_OUTLINE_POLYGON;
}

void guiInit(int argc, char **argv)
{
	int i = 0;

	//sets the mode
	int mode = GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH;
	glutInit(&argc, argv);
	glutInitDisplayMode(mode);
	glutInitWindowSize(Window_Width_section, Window_Height_section);
	glutInitWindowPosition(0, 0);

	//////////////////////////////////// section curve window
	curveWindow = glutCreateWindow("section curve window");

	glutDisplayFunc(displaySection);

	glutKeyboardFunc(keyboardSection);
	glutSpecialFunc(keyboardSectionSpecial);

	glutReshapeFunc(aspectRatioSection);

	glutMouseFunc(mouseSection);					  //callback for buttons events
	glutMotionFunc(mouseMotionSection);				  //callback for active motion events
	glutPassiveMotionFunc(mousePassiveMotionSection); //callback for passive motion events

	GLUI_Master.set_glutIdleFunc(NULL);

	glClearColor(0.4, 0.5, 0.8, 0.0);

	section_ctrlpoint_subMenu = glutCreateMenu(menu);
	glutAddMenuEntry("Add", 1);
	glutAddMenuEntry("Delete", 2);
	glutAddMenuEntry("Quit", 'q');

	glutCreateMenu(menu);
	glutAddSubMenu("Points", section_ctrlpoint_subMenu);
	glutAddMenuEntry("Quit", 'q');
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	//////////////////////////////////// end of the section curve window

	//////////////////////////////////// main window
	glutInitWindowSize(Window_Width, Window_Height);
	glutInitWindowPosition(glutGet(GLUT_SCREEN_WIDTH) - Window_Width, 0);
	main_window_id = glutCreateWindow("main window");

	glutDisplayFunc(display);

	glutReshapeFunc(aspectRatio);

	glutMouseFunc(mouse);		 //callback for buttons events
	glutMotionFunc(mouseMotion); //callback for motion events

	GLUI_Master.set_glutIdleFunc(idle);

	glutKeyboardFunc(keyboard);
	glutSpecialFunc(keyboardSpecial);
	//////////////////////////////////// end of the main window

	init();

	//GLUI window
	glui = GLUI_Master.create_glui("operations");
	glui->set_main_gfx_window(main_window_id);

	//GLUI init

	char buffer2[10] = "";

	cmd_panel = glui->add_panel("commands");
	glui->add_edittext_to_panel(cmd_panel, "file", GLUI_EDITTEXT_TEXT, filename, -1);

	glui->add_button_to_panel(cmd_panel, "load points", -1, (GLUI_Update_CB)load_points);
	glui->add_button_to_panel(cmd_panel, "save points", -1, (GLUI_Update_CB)save_points);

	glui->add_column_to_panel(cmd_panel);

	glui->add_edittext_to_panel(cmd_panel, "background", GLUI_EDITTEXT_TEXT, background, -1);
	glui->add_button_to_panel(cmd_panel, "background", -1, (GLUI_Update_CB)load_background);

	panel = glui->add_panel("operations");
	panel_cols = glui->add_panel_to_panel(panel, "plane");

	multiplicity_panel = glui->add_panel_to_panel(panel, "multiplicity");

	SerializatorFactory::getSerializator()->init_points();

	rotation = glui->add_rotation_to_panel(panel, "rotation", rotation_data, -1, (GLUI_Update_CB)do_rotation);
	z_rot_spinner = glui->add_spinner_to_panel(panel, "z_rot", GLUI_SPINNER_FLOAT, &z_rotation,
											   -1, (GLUI_Update_CB)do_z_rotation);
	z_rot_spinner->set_speed(100.0);
	glui->add_statictext_to_panel(panel, "ctrl: around y");
	glui->add_statictext_to_panel(panel, "alt: around x");
	glui->add_statictext_to_panel(panel, "arrows: around z");
	GLUI_Spinner *weight_spinner = glui->add_spinner_to_panel(panel, "weight", GLUI_SPINNER_FLOAT, &weightSelectedCtrlPoint,
															  -1, (GLUI_Update_CB)do_weight_adjust);
	weight_spinner->set_speed(0.5);

	GLUI_Listbox *lb = glui->add_listbox_to_panel(panel, "surf type", (int *)&newSurfType);
	lb->add_item(Revolving_surfType, "Revolving surface");
	//lb->add_item(Extruded_surfType, "Extruded surface");
	glui->add_button_to_panel(panel, "create new surf", -1, (GLUI_Update_CB)createNewSurf);

	GLUI_Spinner *degree_S_spinner = glui->add_spinner_to_panel(panel, "degree S",
																GLUI_SPINNER_INT, &newDegreeS, -1, (GLUI_Update_CB)changeDegreeS);
	degree_S_spinner->set_speed(1.0);
	GLUI_Spinner *degree_T_spinner = glui->add_spinner_to_panel(panel, "degree T",
																GLUI_SPINNER_INT, &newDegreeT, -1, (GLUI_Update_CB)changeDegreeT);
	degree_T_spinner->set_speed(1.0);

	//second column in panel

	glui->add_column_to_panel(panel);

	x_translation = glui->add_translation_to_panel(panel, "x", GLUI_TRANSLATION_X,
												   transl_x, -1, (GLUI_Update_CB)do_transl_x);
	x_translation->set_speed(0.002);

	y_translation = glui->add_translation_to_panel(panel, "y", GLUI_TRANSLATION_Y,
												   transl_y, -1, (GLUI_Update_CB)do_transl_y);
	y_translation->set_speed(0.002);

	z_translation = glui->add_translation_to_panel(panel, "z", GLUI_TRANSLATION_Z,
												   transl_z, -1, (GLUI_Update_CB)do_transl_z);
	z_translation->set_speed(0.002);

	zy_translation = glui->add_translation_to_panel(panel, "zy", GLUI_TRANSLATION_XY,
													transl_zy, -1, (GLUI_Update_CB)do_transl_zy);
	zy_translation->set_speed(0.002);

	glui->add_button_to_panel(panel, "scaleUp", -1, (GLUI_Update_CB)do_scale_up);
	glui->add_button_to_panel(panel, "scaleDown", -1, (GLUI_Update_CB)do_scale_down);

	glui->add_checkbox_to_panel(panel, "skinning", &display_skinning, -1, (GLUI_Update_CB)skinning);
	spinner_destination = glui->add_spinner_to_panel(panel, "dest. plane",
													 GLUI_SPINNER_INT, &destination_column_copy, -1);
	spinner_destination->set_int_limits(0, NCP_T - 1);

	glui->add_button_to_panel(panel, "copy", -1, (GLUI_Update_CB)copy_column);

	glui->add_checkbox_to_panel(panel, "fill surf", &nurbsDisplayModeInt, -1, (GLUI_Update_CB)setFillNurbs);
	glui->add_checkbox_to_panel(panel, "show pts", &showCtrlPtsPlanes, -1);

	glui->sync_live();
}