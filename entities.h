#ifndef ENTITIES
#define ENTITIES

static const double PI = 3.14159265;

class vector3d
{
public:
      float x;
      float y;
      float z;

      vector3d();
      vector3d(float a, float b, float c);

      vector3d operator+(const vector3d &v) const;
      vector3d operator-(const vector3d &v) const;
      vector3d operator*(const float f) const;
      bool operator==(const vector3d v) const;
      float dotProd(const vector3d &v) const;
      vector3d crossProd(const vector3d &v) const;
      float norm() const;
      vector3d normalize() const;
      void print(char *s);
};

class point3d
{
public:
      float x;
      float y;
      float z;

      point3d();
      point3d(float a, float b, float c);

      point3d operator+(const vector3d &v) const;
      point3d operator-(const vector3d &v) const;
      vector3d operator+(const point3d &v) const;
      vector3d operator-(const point3d &v) const;
      point3d operator*(const float f) const;
      bool operator==(const point3d p) const;
      void pointToLine(point3d S, vector3d T, point3d *Pp);
      float distance(const point3d p) const;
      void print(char *s);
};

int intersect3DLines(point3d p1, vector3d v1, point3d p2, vector3d v2, point3d *Pij);

#endif