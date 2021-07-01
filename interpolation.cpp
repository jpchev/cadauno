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
#include "linearSys.h"
#include "evaluator.h"

GLfloat minimum(int a, int b)
{
	return a > b ? b : a;
}

//computes the norm of a vector
GLfloat norm(GLfloat *v, int dim)
{
	int i;
	GLfloat acc = 0.0, result;
	for (i = 0; i < dim; i++)
		acc += v[i] * v[i];
	result = sqrt(acc);
	return result;
}

/**
* global interpolation through n + 1 points QQ with a spline curve of degree p
* on a given knot vector U
* INPUT: n, QQ, r, p, U, uk
* OUTPUT: PP
*/
void curveinterp_skinning(GLfloat *PP, int n, GLfloat *QQ, int r, int p, GLfloat *U, GLfloat *uk)
{
	int i, j, dim = n + 1;
	GLfloat *sol, *rhs;
	GLfloat *A = new GLfloat[dim * dim];

	// set the coefficient matrix (n + 1) x (n + 1)
	for (i = 0; i < n + 1; i++)
		for (j = 0; j < n + 1; j++)
			A[i * (n + 1) + j] = 0.0;

	int span;
	GLfloat *N = new GLfloat[p + 1];

	for (i = 1; i <= n + 1; i++)
	{
		span = findspan(n, p, uk[i - 1], U);
		basis_Nim(N, span, uk[i - 1], p, U);
		for (j = span - p; j <= minimum(span, n + 1); j++)
			A[(i - 1) * (n + 1) + j - 1] = N[j - (span - p)]; // i-th row
	}

	sol = new GLfloat[dim];
	rhs = new GLfloat[dim];

	//r is the number of coordinates
	for (i = 1; i <= r; i++)
	{
		for (j = 1; j <= n + 1; j++)
		{
			rhs[j - 1] = QQ[(j - 1) * r + i - 1]; //i-th coordinate of Q_j
		}										  //for

		SolveSysGauss(A, rhs, sol, dim);

		for (j = 1; j <= n + 1; j++)
		{
			PP[(j - 1) * r + i - 1] = sol[j - 1]; //i-th coordinate of P_j
		}										  //for
	}											  //for

	delete[] A;
	delete[] N;
	delete[] sol;

} //curvInterpSkinning

/**
* SKINNING SURFACES with non-rational spline
* 
* INPUT: K + 1 section curves (non rational) defined on the same knot vector U
*           and having common degree p and control points P
* OUTPUT: skinning surface of degree (p,q), control points Q, knot vector U,V
*/
void skinning()
{

	int p = S_ORDER - 1; //curve degree on S

	int m = S_NUMKNOTS;	   //number of knots on S
	int n = m - 1 - p - 1; //n + 1 = number of control point in S = NCP_S

	int r = 4, i, j, k;
	int K = NCP_T - 1; // K + 1 section curves, K + 1 = NCP_T

	GLfloat *P = new GLfloat[(K + 1) * (n + 1) * r];
	GLfloat *Q = new GLfloat[(n + 1) * (K + 1) * r];

	for (i = 0; i < NCP_T; i++)
		for (j = 0; j < NCP_S; j++)
			for (k = 0; k < r; k++)
				P[i * (n + 1) * r + j * r + k] = ctrlpoints[i * NCP_S * 4 + 4 * j + k];

	/*
	//print P
	for(k=0;k<3;k++){
		printf("k=%d\n",k);
		for(i=0;i<NCP_T;i++){
			for(j=0;j<NCP_S;j++)printf("%f  ",P[i*(n+1)*r+j*r+k]);
			printf("\n");
		}
		printf("---------\n");
	}
	*/

	// curve degree q (arbitrary but q <= K)
	int q = T_ORDER - 1;

	if (q > K)
		q = K;

	//define K + 1 parameters vk
	//they will be used for the technique of averaging in knots computation along the V direction
	GLfloat *vk = new GLfloat[K + 1];
	GLfloat *d = new GLfloat[n + 1];
	GLfloat *v = new GLfloat[r];

	vk[0] = 0.0;
	vk[K] = 1.0;
	GLfloat sum;
	int w;
	for (k = 2; k <= K; k++)
	{
		sum = 0.0;
		for (i = 1; i <= n + 1; i++)
		{
			d[i - 1] = 0.0;
			for (j = 2; j <= K + 1; j++)
			{
				for (w = 1; w <= r; w++)
				{
					v[w - 1] =
						P[(j - 1) * (n + 1) * r + (i - 1) * r + w - 1] -
						P[(j - 2) * (n + 1) * r + (i - 1) * r + w - 1];
				}
				d[i - 1] += sqrt(norm(v, r));
			} //for
			for (w = 1; w <= r; w++)
			{
				v[w - 1] =
					P[(k - 1) * (n + 1) * r + (i - 1) * r + w - 1] -
					P[(k - 2) * (n + 1) * r + (i - 1) * r + w - 1];
			}
			sum += sqrt(norm(v, r)) / d[i - 1];
		} //for
		vk[k - 1] = vk[k - 2] + ((1.0) / (n + 1)) * sum;
	} //for

	/* compute the knot vector V starting from the vk parameters, begin */
	int mv = K + q + 1;

	GLfloat *V = new GLfloat[mv + 1]; //knots vector in the T direction

	for (i = 1; i <= p + 1; i++)
	{
		V[i - 1] = 0.0; // first p + 1 knots set to 0.0
	}					//for

	for (j = 1; j <= K - q; j++)
	{
		sum = 0.0;

		for (i = j; i <= j + q - 1; i++)
			sum = sum + vk[i];

		V[j + q] = ((1.0) / q) * sum;
	} //for

	for (i = mv - q + 1; i <= mv + 1; i++)
	{
		V[i - 1] = 1.0; // last q + 1 knots set to 1.0
	}
	/* compute the knot vector V, end */

	GLfloat *PP = new GLfloat[(K + 1) * r];

	GLfloat *Qi = new GLfloat[(K + 1) * r];

	//Compute the control points Q of the skinned surface
	//by means of n + 1 curve interpolations
	for (i = 1; i <= n + 1; i++)
	{

		for (j = 0; j < K + 1; j++)
			for (k = 0; k < r; k++)
				PP[j * r + k] = P[j * (n + 1) * r + (i - 1) * r + k];

		curveinterp_skinning(Qi, K, PP, r, q, V, vk);

		for (j = 0; j < K + 1; j++)
			for (k = 0; k < r; k++)
				Q[(i - 1) * (K + 1) * r + j * r + k] = Qi[j * r + k];
	} //for

	/*
	//print Q
	for(k=0;k<3;k++){
		printf("k=%d\n",k);
		for(i=0;i<NCP_S;i++){
			for(j=0;j<NCP_T;j++)printf("%f  ",Q[i*(K+1)*r+j*r+k]);
			printf("\n");
		}
		printf("---------\n");
	}
	*/

	//model update
	delete[] skinning_ctrlpoints;
	skinning_ctrlpoints = new GLfloat[NCP_S * NCP_T * 4];
	delete[] skinning_tknots;
	skinning_tknots = new GLfloat[T_NUMKNOTS];

	for (j = 0; j < NCP_T; j++)
	{
		for (i = 0; i < NCP_S; i++)
		{
			for (k = 0; k < 4; k++)
			{
				if (k >= r)
					skinning_ctrlpoints[j * 4 * NCP_S + i * 4 + k] = 1.0;
				else
					skinning_ctrlpoints[j * 4 * NCP_S + i * 4 + k] = Q[i * (K + 1) * r + j * r + k];
			}
		}
	}

	for (i = 0; i < T_NUMKNOTS; i++)
	{
		skinning_tknots[i] = V[i];
	}

	glui->sync_live();
	display();

	delete[] P;
	delete[] Q;
	delete[] Qi;
	delete[] vk;
	delete[] d;
	delete[] PP;
	delete[] v;
	delete[] V;

} //skinning