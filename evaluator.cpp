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
#include "include/glui.h"
#include "extern.h"

/**
* computes the p + 1 basis functions base not null N_[i,p](u) in u
% INPUT: i, u, p, U
% p degree
% u parameter
% OUTPUT: vector N
*/
void basis_Nim(GLfloat *N, int i, GLfloat u, int p, GLfloat *U)
{
	int r, j;
	GLfloat saved, temp;
	GLfloat *left = new float[p];
	GLfloat *right = new float[p];
	i = i - 1;
	N[0] = 1.0;
	for (j = 1; j <= p; j++)
	{
		left[j - 1] = u - U[i + 1 - j];
		right[j - 1] = U[i + j] - u;
		saved = 0.0;
		for (r = 0; r <= j - 1; r++)
		{
			temp = N[r] / (right[r] + left[j - r - 1]);
			N[r] = saved + right[r] * temp;
			saved = left[j - r - 1] * temp;
		} //for
		N[j] = saved;
	} //for

	delete[] left;
	delete[] right;

} //basis_Nim

int fix(double num)
{
	if (num >= 0)
		return floor(num);
	else
		return -floor(-num);
}

int findspan(int n, int p, float u, float *U)
{
	int ind, low, high, mid;
	if (u == U[n + 1])
		ind = n + 1;
	else
	{
		low = p + 1;
		high = n + 1 + 1;
		//binary search
		mid = fix((low + high) / 2.0);
		while ((u < U[mid - 1]) | (u >= U[mid]))
		{
			if (u < U[mid - 1])
			{
				high = mid;
			}
			else
			{
				low = mid;
			} //else
			mid = fix((low + high) / 2.0);
			if ((high - low) <= 1)
				break;
		} //while
		ind = mid;
	} //else

	return ind;

} //findspan

void eval_surf(GLfloat *x, GLfloat *y, GLfloat *z, GLfloat u, GLfloat v)
{
	int n, m, i_s = 0, i_t = 0, i, j, sk, tk;
	n = S_ORDER;
	m = T_ORDER;
	GLfloat num_x = 0.0, num_y = 0.0, num_z = 0.0,
			den_x = 0.0, den_y = 0.0, den_z = 0.0;
	GLfloat *N_S = new GLfloat[S_ORDER];
	GLfloat *N_T = new GLfloat[T_ORDER];

	i_s = findspan(NCP_S - 1, n - 1, u, sknots);
	i_t = findspan(NCP_T - 1, m - 1, v, tknots);
	basis_Nim(N_S, i_s, u, n - 1, sknots);
	basis_Nim(N_T, i_t, v, m - 1, tknots);

	for (tk = 0, i = i_t - m + 1; i <= i_t; i++, tk++)
	{
		for (sk = 0, j = i_s - n + 1; j <= i_s; j++, sk++)
		{
			//the multiply by weights has already been done in ctrlpoints
			num_x = num_x + N_S[sk] * N_T[tk] * ctrlpoints[(i - 1) * NCP_S * 4 + (j - 1) * 4];
			den_x = den_x + N_S[sk] * N_T[tk] * ctrlpoints[(i - 1) * NCP_S * 4 + (j - 1) * 4 + 3];

			num_y = num_y + N_S[sk] * N_T[tk] * ctrlpoints[(i - 1) * NCP_S * 4 + (j - 1) * 4 + 1];
			den_y = den_y + N_S[sk] * N_T[tk] * ctrlpoints[(i - 1) * NCP_S * 4 + (j - 1) * 4 + 3];

			num_z = num_z + N_S[sk] * N_T[tk] * ctrlpoints[(i - 1) * NCP_S * 4 + (j - 1) * 4 + 2];
			den_z = den_z + N_S[sk] * N_T[tk] * ctrlpoints[(i - 1) * NCP_S * 4 + (j - 1) * 4 + 3];
		} //for
	}	  //for

	*x = num_x / den_x;
	*y = num_y / den_y;
	*z = num_z / den_z;

	delete[] N_S;
	delete[] N_T;
}