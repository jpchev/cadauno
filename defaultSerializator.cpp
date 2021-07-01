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

#include <stdio.h>
#include "include/glui.h"
#include "extern.h"

#include "serializator.h"

//needed for a weird C++ compiler behavior (it shouldn't logically be necessary)
Serializator *SerializatorFactory::ser;

Serializator *ser = SerializatorFactory::getSerializator();

///////////////// list of "delegates" functions rerouting the callback
///////////////// to member functions of the choosen Serializator
void load_points(int id)
{
	ser->load_points(id);
}
void save_points(int id)
{
	ser->save_points(id);
}
///////////////// end of list of "delegates" functions rerouting the callback
///////////////// to member functions of the choosen Serializator

DefaultSerializator::DefaultSerializator() {}

void DefaultSerializator::load_points(int id)
{
	char buffer[50];
	GLfloat *file_ctrlpoints, *file_sknots = NULL, *file_tknots = NULL;
	int sdegree = -1, tdegree = -1, ncp_s = -1, ncp_t = -1, s_numknots = -1, t_numknots = -1;
	FILE *fpo = fopen(filename, "r");
	int i;
	if (fpo == NULL)
		return;
	if (fscanf(fpo, "%s", buffer) == EOF)
		return; /*FILENAME:...*/
	if (fscanf(fpo, "%s", buffer) == EOF)
		return; /*DEGREE...*/
	if (fscanf(fpo, "%d", &sdegree) == EOF)
		return; /*S_ORDER-1*/
	if (fscanf(fpo, "%d", &tdegree) == EOF)
		return; /*T_ORDER-1*/
	if (fscanf(fpo, "%s", buffer) == EOF)
		return; /*NCP_U_V:...*/
	if (fscanf(fpo, "%d", &ncp_s) == EOF)
		return; /*NCP_S:...*/
	if (fscanf(fpo, "%d", &ncp_t) == EOF)
		return; /*NCP_T:...*/

	if (fscanf(fpo, "%s", buffer) == EOF)
		return; /*NKNOTS_UV:...*/
	if (fscanf(fpo, "%d", &s_numknots) == EOF)
		return; /*s_numknots:...*/
	if (fscanf(fpo, "%d", &t_numknots) == EOF)
		return; /*t_numknots:...*/

	if (s_numknots != (ncp_s + sdegree + 1))
		return;
	if (t_numknots != (ncp_t + tdegree + 1))
		return;

	if (fscanf(fpo, "%s", buffer) == EOF)
		return; /*COORD_CP:...*/
	file_ctrlpoints = new GLfloat[ncp_s * ncp_t * 4];
	for (i = 0; i < ncp_s * ncp_t * 4; i++)
	{
		if (fscanf(fpo, "%f", file_ctrlpoints + i) == EOF)
		{
			if (file_ctrlpoints != NULL)
			{
				delete[] file_ctrlpoints;
				return;
			}
		} //if
	}	  //for

	S_ORDER = sdegree + 1;
	T_ORDER = tdegree + 1;

	newDegreeS = sdegree;
	newDegreeT = tdegree;

	S_NUMKNOTS = s_numknots;
	T_NUMKNOTS = t_numknots;

	NCP_S = ncp_s;
	NCP_T = ncp_t;

	//multiplicity = T_ORDER - 1;
	multiplicity = 1;

	if (skinning_tknots != NULL)
		delete[] skinning_tknots;

	skinning_tknots = new GLfloat[T_NUMKNOTS];

	if (skinning_ctrlpoints != NULL)
		delete[] skinning_ctrlpoints;

	skinning_ctrlpoints = new GLfloat[NCP_S * NCP_T * 4];

	fscanf(fpo, "%s", buffer); /*KNOTS_U:...*/
	if (strcmp(buffer, "KNOTS_U") == 0)
	{
		file_sknots = new GLfloat[s_numknots];
		for (i = 0; i < s_numknots; i++)
		{
			if (fscanf(fpo, "%f", file_sknots + i) == EOF)
			{
				if (file_ctrlpoints != NULL)
				{
					delete[] file_ctrlpoints;
					delete[] file_sknots;
					return;
				} //if
			}	  //if
		}		  //for
		if (sknots != NULL)
			delete[] sknots;

		sknots = new GLfloat[S_NUMKNOTS];
		for (i = 0; i < S_NUMKNOTS; i++)
			sknots[i] = file_sknots[i];
		fscanf(fpo, "%s", buffer); /*KNOTS_V:...*/
	}
	else
		knots_uniform(1);

	if (strcmp(buffer, "KNOTS_V") == 0)
	{
		file_tknots = new GLfloat[t_numknots];
		for (i = 0; i < t_numknots; i++)
		{
			if (fscanf(fpo, "%f", file_tknots + i) == EOF)
			{
				if (file_ctrlpoints != NULL)
					delete[] file_ctrlpoints;
				if (file_sknots != NULL)
					delete[] file_sknots;
				if (file_tknots != NULL)
				{
					delete[] file_tknots;
					return;
				}
			}
		} //for
		if (tknots != NULL)
			delete[] tknots;
		tknots = new GLfloat[T_NUMKNOTS];
		for (i = 0; i < T_NUMKNOTS; i++)
			tknots[i] = file_tknots[i];
	} //if
	else
		knots_uniform(2);

	if (ctrlpoints != NULL)
		delete[] ctrlpoints;
	ctrlpoints = new GLfloat[NCP_S * NCP_T * 4];

	for (i = 0; i < NCP_S * NCP_T * 4; i++)
		ctrlpoints[i] = file_ctrlpoints[i];

	delete[] multiple_tknots;
	multiple_tknots = NULL;
	delete[] multiple_ctrlpoints;
	multiple_ctrlpoints = NULL;

	fclose(fpo);
	if (file_ctrlpoints != NULL)
		delete[] file_ctrlpoints;
	if (file_sknots != NULL)
		delete[] file_sknots;
	if (file_tknots != NULL)
		delete[] file_tknots;

	//GLUI update
	gluiSurfaceParametersInit();

	glui->sync_live();

} //load_points

void DefaultSerializator::save_points(int id)
{
	FILE *fpo = fopen(filename, "w");
	int i;
	if (fpo == NULL)
		return;
	fprintf(fpo, "FILENAME:%s\n", filename);

	fprintf(fpo, "DEGREE_U_V\n");
	fprintf(fpo, "     %d     %d\n", S_ORDER - 1, T_ORDER - 1);

	fprintf(fpo, "N.C.P._U_V\n");
	fprintf(fpo, "     %d     %d\n", NCP_S, NCP_T);

	fprintf(fpo, "N.KNOTS_U_V\n");
	fprintf(fpo, "     %d     %d\n", S_NUMKNOTS, T_NUMKNOTS);

	fprintf(fpo, "COORD.C.P.(X,Y,Z,W)\n");
	if (display_skinning != 0)
	{
		for (i = 0; i < NCP_S * NCP_T * 4; i = i + 4)
		{
			fprintf(fpo, "%f      %f      %f      %f\n",
					skinning_ctrlpoints[i], skinning_ctrlpoints[i + 1],
					skinning_ctrlpoints[i + 2], skinning_ctrlpoints[i + 3]);
			if (((i + 3) % (NCP_S * 4)) == (NCP_S * 4 - 1))
				fprintf(fpo, "\n\n\n");
		} //for
	}
	else
	{
		for (i = 0; i < NCP_S * NCP_T * 4; i = i + 4)
		{
			fprintf(fpo, "%f      %f      %f      %f\n",
					ctrlpoints[i], ctrlpoints[i + 1], ctrlpoints[i + 2], ctrlpoints[i + 3]);
			if (((i + 3) % (NCP_S * 4)) == (NCP_S * 4 - 1))
				fprintf(fpo, "\n\n\n");
		} //for
	}

	fprintf(fpo, "\n\n\nKNOTS_U\n");
	for (i = 0; i < S_NUMKNOTS; i++)
		fprintf(fpo, "%f\n", sknots[i]);

	fprintf(fpo, "\n\n\nKNOTS_V\n");
	for (i = 0; i < T_NUMKNOTS; i++)
		fprintf(fpo, "%f\n", tknots[i]);

	fclose(fpo);
}