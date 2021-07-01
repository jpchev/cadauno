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

#ifndef EVALUATOR
#define EVALUATOR

#include "extern.h"

void basis_Nim(GLfloat *N, int i, GLfloat u, int p, GLfloat *U);

int findspan(int n, int p, float u, float *U);

void eval_surf(GLfloat *x, GLfloat *y, GLfloat *z, GLfloat u, GLfloat v);

#endif