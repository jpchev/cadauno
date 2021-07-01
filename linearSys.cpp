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
#include <GL/glut.h>

void FattGaussPivot(GLfloat *a, GLfloat *b, GLfloat *l, GLfloat *r, int dim)
{
	int i, j, k, line;
	GLfloat max, aus, ass;

	for (i = 0; i < dim; i++)
		for (j = 0; j < dim; j++)
			r[i * dim + j] = a[i * dim + j];
	k = 1;
	while (k < dim)
	{
		max = 0.0;
		line = k;
		for (i = k; i <= dim; i++)
		{
			ass = fabs(r[(i - 1) * dim + k - 1]);
			if (max < ass)
			{
				max = ass;
				line = i;
			}
		}
		if (line != k)
		{
			for (j = k; j <= dim; j++)
			{
				aus = r[(k - 1) * dim + j - 1];
				r[(k - 1) * dim + j - 1] = r[(line - 1) * dim + j - 1];
				r[(line - 1) * dim + j - 1] = aus;
			}
			aus = b[k - 1];
			b[k - 1] = b[line - 1];
			b[line - 1] = aus;
		}
		for (i = k + 1; i <= dim; i++)
		{
			l[(i - 1) * dim + k - 1] = r[(i - 1) * dim + k - 1] / r[(k - 1) * dim + k - 1];
			for (j = k; j <= dim; j++)
			{
				r[(i - 1) * dim + j - 1] =
					r[(i - 1) * dim + j - 1] - l[(i - 1) * dim + k - 1] * r[(k - 1) * dim + j - 1];
			}
			b[i - 1] =
				b[i - 1] - l[(i - 1) * dim + k - 1] * b[k - 1];
		}
		k++;
	}
}

void SolveBack(GLfloat *r, GLfloat *b, GLfloat *x, int dim)
{
	int i, j;
	GLfloat sum;
	x[dim - 1] = b[dim - 1] / r[(dim - 1) * dim + dim - 1];
	for (i = dim - 1; i >= 1; i--)
	{
		sum = 0.0;
		for (j = i + 1; j <= dim; j++)
		{
			sum = sum + r[(i - 1) * dim + j - 1] * x[j - 1];
		}
		x[i - 1] = (b[i - 1] - sum) / r[(i - 1) * dim + i - 1];
	}
}

/***********************************************************************
Solution of a linear system in the form
ax = b
where a and b are matrices.
Gauss method with pivoting is used
************************************************************************/
void SolveSysGauss(GLfloat *a, GLfloat *b, GLfloat *x, int dim)
{
	GLfloat *l, *r;
	l = new GLfloat[dim * dim];
	r = new GLfloat[dim * dim];

	FattGaussPivot(a, b, l, r, dim);

	/*
	//print r
	int i, j;
	for(i=0; i<dim; i++){
		printf("\n");
		for(j=0; j<dim; j++)
		printf("%f  ", r[i*dim+j]);
	}
	*/

	SolveBack(r, b, x, dim);

	delete[] l;
	delete[] r;
}
/**************************************************************/