#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "entities.h"

/**
creates a revolving surface about an axis, with a certain rotation angle
the output surface will use 9 ctrl points (thus 12 knots) in the T direction
*/
void makeRevolvedSurf(
	/*input*/
	point3d S,	//point
	vector3d T, //unit length vector
	float theta /*angle in radians*/, int m, point3d *Pj, float *wj,

	/*output*/
	int *n, float *U, point3d *Pij, float *wij)
{

	int narcs = 0, j = 0;
	float dtheta = 0.0;
	vector3d X, Y, T0, T2;

	point3d P0, P2;

	if (theta <= PI / 2.0)
		narcs = 1;
	else if (theta <= PI)
	{
		narcs = 2;
		U[3] = U[4] = 0.5;
	}
	else if (theta <= PI * 3.0 / 2.0)
	{
		narcs = 3;
		U[3] = U[4] = 1.0 / 3.0;
		U[5] = U[6] = 2.0 / 3.0;
	}
	else
	{
		narcs = 4;
		U[3] = U[4] = 0.25;
		U[5] = U[6] = 0.5;
		U[7] = U[8] = 0.75;
	}

	dtheta = theta / narcs;
	j = 3 + 2 * (narcs - 1); /*load end knots*/

	for (int i = 0; i < 3; j++, i++)
	{
		U[i] = 0.0;
		U[j] = 1.0;
	}

	(*n) = 2 * narcs;

	float wm = cos(dtheta / 2.0); /*dtheta/2 is base angle*/
	float angle = 0.0;			  /*compute sine and cosine only once*/
	float *cosines = (float *)malloc(narcs * sizeof(float));
	float *sines = (float *)malloc(narcs * sizeof(float));

	for (int i = 0; i < narcs; i++)
	{
		angle = angle + dtheta;
		cosines[i] = cos(angle);
		sines[i] = sin(angle);
	}
	float r = 0.0;

	for (int j = 0; j <= m; j++)
	{
		/*loop and compute each u row of ctrl pts and weigths*/
		point3d Or;
		(Pj + j)->pointToLine(S, T, &Or);

		X = Pj[j] - Or;

		r = X.norm();
		X = X.normalize();

		Y = T.crossProd(X);

		/*initialize first*/
		Pij[j] = P0 = Pj[j];

		wij[j] = wj[j]; /*ctrl pt and weight*/

		T0 = Y;
		int index = 0;
		angle = 0.0;

		for (int i = 0; i < narcs; i++) /*compute u row*/
		{
			P2 = Or + X * r * cosines[i] + Y * r * sines[i];

			Pij[(index + 2) * (m + 1) + j] = P2;
			wij[(index + 2) * (m + 1) + j] = wj[j];

			T2 = X * -sines[i] + Y * cosines[i];

			intersect3DLines(P0, T0, P2, T2, &Pij[(index + 1) * (m + 1) + j]);

			wij[(index + 1) * (m + 1) + j] = wm * wj[j];
			index = index + 2;
			if (i < narcs - 1)
			{
				P0 = P2;
				T0 = T2;
			}
		}
	}
	free(cosines);
	free(sines);
}