#ifndef PLANE_H
#define PLANE_H


#include "Vector3D.h"
#include "export.h"


struct EXPORTED Plane {
    Vector3D normal;
    float w;

    Plane();

    Plane(float nx, float ny, float nz, float d);

    Plane(const Vector3D &n, float d);
};

EXPORTED float dot_vector(const Plane &plane, const Vector3D &v);

EXPORTED float dot_point(const Plane &plane, const Vector3D &p);


#endif //PLANE_H
