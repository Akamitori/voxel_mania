#ifndef PLANE_H
#define PLANE_H


#include "Vector3D.h"
#include "linearalgebra_export.h"


struct LINEARALGEBRA_EXPORT Plane {
    Vector3D normal;
    float w;

    Plane();

    Plane(float nx, float ny, float nz, float d);

    Plane(const Vector3D &n, float d);
};

LINEARALGEBRA_EXPORT float dot_vector(const Plane &plane, const Vector3D &v);

LINEARALGEBRA_EXPORT float dot_point(const Plane &plane, const Vector3D &p);


#endif //PLANE_H
