#include <stdio.h>
#include <math.h>
#include "entities.h"

point3d::point3d() {}

point3d::point3d(float a, float b, float c)
{
  x = a;
  y = b;
  z = c;
}

point3d point3d::operator+(const vector3d &v) const
{
  point3d res;
  res.x = x + v.x;
  res.y = y + v.y;
  res.z = z + v.z;
  return res;
}

point3d point3d::operator-(const vector3d &v) const
{
  point3d res;
  res.x = x - v.x;
  res.y = y - v.y;
  res.z = z - v.z;
  return res;
}

float point3d::distance(const point3d p) const
{
  return sqrt(
      (p.x - x) * (p.x - x) +
      (p.y - y) * (p.y - y) +
      (p.z - z) * (p.z - z));
}

void point3d::print(char *s)
{
  sprintf(s, "x:%f, y:%f, z:%f\r\n", x, y, z);
}

vector3d::vector3d() {}

bool vector3d::operator==(const vector3d v) const
{
  return x == v.x && y == v.y && z == v.z;
}

vector3d::vector3d(float a, float b, float c)
{
  x = a;
  y = b;
  z = c;
}

bool point3d::operator==(const point3d p) const
{
  return x == p.x && y == p.y && z == p.z;
}

vector3d point3d::operator+(const point3d &v) const
{
  vector3d res;
  res.x = x + v.x;
  res.y = y + v.y;
  res.z = z + v.z;
  return res;
}

vector3d point3d::operator-(const point3d &v) const
{
  vector3d res;
  res.x = x - v.x;
  res.y = y - v.y;
  res.z = z - v.z;
  return res;
}

point3d point3d::operator*(const float f) const
{
  point3d res;
  res.x = x * f;
  res.y = y * f;
  res.z = z * f;
  return res;
}

void point3d::pointToLine(point3d S, vector3d T, point3d *Pp)
{
  //Pp = A + {(AB � AP) / || AB ||�} AB
  point3d P(x, y, z);
  point3d A = S;
  point3d B = S + T;
  vector3d AB = B - A;
  vector3d AP = P - A;
  float AB_norm = AB.norm();
  float t = AB.dotProd(AP) / (AB_norm * AB_norm);
  *Pp = A + AB * t;
}

vector3d vector3d::operator+(const vector3d &v) const
{
  vector3d res;
  res.x = x + v.x;
  res.y = y + v.y;
  res.z = z + v.z;
  return res;
}

vector3d vector3d::operator-(const vector3d &v) const
{
  vector3d res;
  res.x = x - v.x;
  res.y = y - v.y;
  res.z = z - v.z;
  return res;
}

vector3d vector3d::operator*(const float f) const
{
  vector3d res;
  res.x = x * f;
  res.y = y * f;
  res.z = z * f;
  return res;
}

float vector3d::dotProd(const vector3d &v) const
{
  return x * v.x + y * v.y + z * v.z;
}

vector3d vector3d::crossProd(const vector3d &v) const
{
  //a � b = (a2b3 - a3b2) i + (a3b1 - a1b3) j + (a1b2 - a2b1) k
  vector3d res;
  res.x = y * v.z - z * v.y;
  res.y = z * v.x - x * v.z;
  res.z = x * v.y - y * v.x;
  return res;
}

float vector3d::norm() const
{
  return sqrt(x * x + y * y + z * z);
}

vector3d vector3d::normalize() const
{
  float norm = this->norm();
  if (norm == 0)
    return *this;
  vector3d res;
  res.x = x / norm;
  res.y = y / norm;
  res.z = z / norm;
  return res;
}

void vector3d::print(char *s)
{
  sprintf(s, "x:%f y:%f z:%f\r\n", x, y, z);
}

int intersect3DLines(point3d p1, vector3d v1, point3d p2, vector3d v2, point3d *Pij)
{
  /*
	L1: (px1 + vx1 * t, py1 + vy1 * t, pz1 + vz1 * t)
	L2: (px2 + vx2 * s, py2 + vy2 * s, pz2 + vz2 * s)



    first attempt
	{px1 + vx1 * t = px2 + vx2 * s
	{py1 + vy1 * t = py2 + vy2 * s

    using the Cramer method:
          
    {vx1 * t - vx2 * s = px2 - px1
    {vy1 * t - vy2 * s = py2 - py1
    
    det = - vx1 * vy2 + vx2 * vy1
    if(det != 0){
        t = (- (px2 - px1) * vy2 + vx2 * (py2 - py1)) / det
        s = (vx1 * (py2 - py1) - vy1 * (px2 - px1)) / det
    
    	check if it holds also for the following
    	pz1 + vz1 * t == pz2 + vz2 * s
    	
    	if holds then return
    }
    
    
    
    second attempt
	{px1 + vx1 * t = px2 + vx2 * s
	{pz1 + vz1 * t = pz2 + vz2 * s

    using the Cramer method:
          
    {vx1 * t - vx2 * s = px2 - px1
    {vz1 * t - vz2 * s = pz2 - pz1
    
    det = - vx1 * vz2 + vx2 * vz1
    if(det != 0){
        t = (- (px2 - px1) * vz2 + vx2 * (pz2 - pz1)) / det
        s = (vx1 * (pz2 - pz1) - vz1 * (px2 - px1)) / det
    
    	check if it holds also for the following
    	py1 + vy1 * t == py2 + vy2 * s
    	
    	if holds then return
    }
    
    
    
    third attempt
	{py1 + vy1 * t = py2 + vy2 * s
	{pz1 + vz1 * t = pz2 + vz2 * s

    using the Cramer method:
          
    {vx1 * t - vx2 * s = px2 - px1
    {vz1 * t - vz2 * s = pz2 - pz1
    
    det = - vx1 * vz2 + vx2 * vz1
    if(det != 0){
        t = (- (px2 - px1) * vz2 + vx2 * (pz2 - pz1)) / det
        s = (vx1 * (pz2 - pz1) - vz1 * (px2 - px1)) / det
    
    	check if it holds also for the following
    	px1 + vx1 * t == px2 + vy2 * s
    	
    	if holds then return
    }
    
    
	*/
  float px1 = p1.x;
  float py1 = p1.y;
  float pz1 = p1.z;

  float px2 = p2.x;
  float py2 = p2.y;
  float pz2 = p2.z;

  float vx1 = v1.x;
  float vy1 = v1.y;
  float vz1 = v1.z;

  float vx2 = v2.x;
  float vy2 = v2.y;
  float vz2 = v2.z;

  float det = 0.0, s = 0.0, t = 0.0;

  //first attempt, columns 1 and 2
  det = -vx1 * vy2 + vx2 * vy1;
  if (det != 0)
  {
    t = (-(px2 - px1) * vy2 + vx2 * (py2 - py1)) / det;
    s = (vx1 * (py2 - py1) - vy1 * (px2 - px1)) / det;

    if (pz1 + vz1 * t == pz2 + vz2 * s)
    {
      *Pij = p1 + v1 * t;
      return 1;
    }
  }

  //second attempt, columns 1 and 3
  det = -vx1 * vz2 + vx2 * vz1;
  if (det != 0)
  {
    t = (-(px2 - px1) * vz2 + vx2 * (pz2 - pz1)) / det;
    s = (vx1 * (pz2 - pz1) - vz1 * (px2 - px1)) / det;

    if (py1 + vy1 * t == py2 + vy2 * s)
    {
      *Pij = p1 + v1 * t;
      return 1;
    }
  }

  //third attempt, columns 2 and 3
  det = -vx1 * vz2 + vx2 * vz1;
  if (det != 0)
  {
    t = (-(px2 - px1) * vz2 + vx2 * (pz2 - pz1)) / det;
    s = (vx1 * (pz2 - pz1) - vz1 * (px2 - px1)) / det;

    if (px1 + vx1 * t == px2 + vy2 * s)
    {
      *Pij = p1 + v1 * t;
      return 1;
    }
  }

  if (p1 == p2)
  {
    *Pij = p1;
    return 1;
  }

  printf("ERROR, lines not intersecting\r\n");
  char str[50];
  p1.print(str);
  printf(str);

  v1.print(str);
  printf(str);

  p2.print(str);
  printf(str);

  v2.print(str);
  printf(str);

  return 0;
}