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

void multiply_matrix(GLfloat *m1, GLfloat *m2, GLfloat *result)
{
	int i = 0;
	for (i = 0; i < 4; i++)
	{
		result[i] = 0;
	}
	for (i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			*(result + i) += m1[i * 4 + j] * m2[j];
		} //for
	}	  //for
}

void do_scale_up(void)
{
	for (int i = 0; i < NCP_S; i++)
	{

		//subtract barycentre
		for (int v = 0; v < 3; v++)
		{
			ctrlpoints[selected_column * 4 * NCP_S + i * 4 + v] =
				ctrlpoints[selected_column * 4 * NCP_S + i * 4 + v] - barycentre[v] *
																		  ctrlpoints[selected_column * 4 * NCP_S + i * 4 + 3]; //weight
		}

		for (int v = 0; v < 3; v++)
		{
			ctrlpoints[selected_column * 4 * NCP_S + i * 4 + v] *= 1.25; //scale up
		}

		//restore barycentre
		for (int v = 0; v < 3; v++)
		{
			ctrlpoints[selected_column * 4 * NCP_S + i * 4 + v] =
				ctrlpoints[selected_column * 4 * NCP_S + i * 4 + v] + barycentre[v] *
																		  ctrlpoints[selected_column * 4 * NCP_S + i * 4 + 3]; //weight
		}
	}
	if (display_skinning != 0)
	{
		skinning();
	}
}

void do_scale_down(void)
{
	for (int i = 0; i < NCP_S; i++)
	{
		//subtract barycentre
		for (int v = 0; v < 3; v++)
		{
			ctrlpoints[selected_column * 4 * NCP_S + i * 4 + v] =
				ctrlpoints[selected_column * 4 * NCP_S + i * 4 + v] - barycentre[v] *
																		  ctrlpoints[selected_column * 4 * NCP_S + i * 4 + 3]; //weight
		}

		for (int v = 0; v < 3; v++)
		{
			ctrlpoints[selected_column * 4 * NCP_S + i * 4 + v] *= 0.8; //scale down
		}

		//restore barycentre
		for (int v = 0; v < 3; v++)
		{
			ctrlpoints[selected_column * 4 * NCP_S + i * 4 + v] =
				ctrlpoints[selected_column * 4 * NCP_S + i * 4 + v] + barycentre[v] *
																		  ctrlpoints[selected_column * 4 * NCP_S + i * 4 + 3]; //weight
		}
	}
	if (display_skinning != 0)
		skinning();
}

void do_transl_x(void)
{
	GLfloat dx = transl_x[0] - old_transl_x[0];
	old_transl_x[0] = transl_x[0];
	for (int i = 0; i < NCP_S; i++)
	{
		ctrlpoints[selected_column * 4 * NCP_S + i * 4] += dx *
														   // point weight
														   ctrlpoints[selected_column * 4 * NCP_S + i * 4 + 3];
	}
	if (display_skinning != 0)
	{
		skinning();
	}
}

void do_transl_y(void)
{
	GLfloat dy = transl_y[0] - old_transl_y[0];
	old_transl_y[0] = transl_y[0];
	for (int i = 0; i < NCP_S; i++)
	{
		ctrlpoints[selected_column * 4 * NCP_S + i * 4 + 1] += dy *
															   //point weight
															   ctrlpoints[selected_column * 4 * NCP_S + i * 4 + 3];
	}
	if (display_skinning != 0)
	{
		skinning();
	}
}

void do_transl_z(void)
{
	GLfloat dz = transl_z[0] - old_transl_z[0];
	old_transl_z[0] = transl_z[0];
	for (int i = 0; i < NCP_S; i++)
	{
		ctrlpoints[selected_column * 4 * NCP_S + i * 4 + 2] += -dz *
															   //point weight
															   ctrlpoints[selected_column * 4 * NCP_S + i * 4 + 3];
	}
	if (display_skinning != 0)
	{
		skinning();
	}
}

void do_transl_zy(void)
{
	GLfloat dz = transl_zy[0] - old_transl_zy[0];
	GLfloat dy = transl_zy[1] - old_transl_zy[1];
	old_transl_zy[0] = transl_zy[0];
	old_transl_zy[1] = transl_zy[1];
	int i;
	for (i = 0; i < NCP_S; i++)
	{
		ctrlpoints[selected_column * 4 * NCP_S + i * 4 + 2] += dz *
															   //point weight
															   ctrlpoints[selected_column * 4 * NCP_S + i * 4 + 3];
	}
	for (i = 0; i < NCP_S; i++)
	{
		ctrlpoints[selected_column * 4 * NCP_S + i * 4 + 1] += dy *
															   //point weight
															   ctrlpoints[selected_column * 4 * NCP_S + i * 4 + 3];
	}
	if (display_skinning != 0)
	{
		skinning();
	}
}

void do_rotation(void)
{
	GLfloat point[4],
		result_point_to_O[4],
		result_point_after_rotation[4],
		final_result_point[4],
		to_O_matrix[16],
		inverse_to_O_matrix[16];

	int i = 0, j = 0;

	for (i = 0; i < 16; i++)
	{
		if ((i % 4 == 3) && (i != 15))
		{
			to_O_matrix[i] = -barycentre[i / 4];
			inverse_to_O_matrix[i] = barycentre[i / 4];
		}
		else if ((i % 4) == (i / 4))
		{
			to_O_matrix[i] = 1.0;
			inverse_to_O_matrix[i] = 1.0;
		}
		else
		{
			to_O_matrix[i] = 0.0;
			inverse_to_O_matrix[i] = 0.0;
		}
	}

	for (i = 0; i < NCP_S; i++)
	{

		for (j = 0; j < 4; j++)
		{
			point[j] = ctrlpoints[selected_column * 4 * NCP_S + i * 4 + j];
		}

		//puts the rotation matrix in the classical form
		rotation_data[4] *= -1.0;
		rotation_data[1] *= -1.0;
		rotation_data[8] *= -1.0;
		rotation_data[2] *= -1.0;
		rotation_data[9] *= -1.0;
		rotation_data[6] *= -1.0;

		multiply_matrix(to_O_matrix, point, result_point_to_O);
		multiply_matrix(rotation_data, result_point_to_O, result_point_after_rotation);
		multiply_matrix(inverse_to_O_matrix, result_point_after_rotation, final_result_point);

		//restores the internal state of the rotation matrix
		rotation_data[4] *= -1.0;
		rotation_data[1] *= -1.0;
		rotation_data[8] *= -1.0;
		rotation_data[2] *= -1.0;
		rotation_data[9] *= -1.0;
		rotation_data[6] *= -1.0;

		for (j = 0; j < 4; j++)
		{
			ctrlpoints[selected_column * 4 * NCP_S + i * 4 + j] =
				final_result_point[j];
		}

	} //for

	rotation->reset();
	if (display_skinning != 0)
	{
		skinning();
	}
}

void do_z_rotation(void)
{
	rotation_data[0] = rotation_data[5] = cos(z_rotation);
	rotation_data[1] = -(rotation_data[4] = sin(z_rotation));
	rotation_data[2] = rotation_data[3] = rotation_data[6] = rotation_data[7] =
		rotation_data[8] = rotation_data[9] = rotation_data[11] = rotation_data[12] =
			rotation_data[13] = rotation_data[14] = 0.0;
	rotation_data[10] = rotation_data[15] = 1.0;
	do_rotation();
	z_rotation = 0;
	z_rot_spinner->set_float_val(0.0);
	if (display_skinning != 0)
	{
		skinning();
	}
}