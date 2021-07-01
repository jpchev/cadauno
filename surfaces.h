#ifndef SURFACES
#define SURFACES

void makeRevolvedSurf(
	/*input*/
	point3d S,	//point
	vector3d T, //unit length vector
	float theta /*angle in radians*/, int m, point3d *Pj, float *wj,

	/*output*/
	int *n, float *U, point3d *Pij, float *wij);

#endif