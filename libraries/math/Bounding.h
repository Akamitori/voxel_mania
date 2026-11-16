#ifndef BOUNDING_H
#define BOUNDING_H

#include "linearalgebra_export.h"

#include <stdint.h>
#include "Vector3D.h"

struct Vector3D;
struct Sphere;
struct Plane;

struct AABB {
    Vector3D center;
    Vector3D size;
};

struct OBB {
    Vector3D center;
    Vector3D size;
    Vector3D axis[3];
};

struct Sphere {
    Vector3D center;
    float radius;
};


LINEARALGEBRA_EXPORT Sphere calculate_bounding_sphere(int32_t vertex_count, const Vector3D *vertices);

LINEARALGEBRA_EXPORT AABB calculate_axis_aligned_bounding_box(int32_t vertex_count, const Vector3D *vertices);

LINEARALGEBRA_EXPORT OBB calculate_oriented_bounding_box(int32_t vertex_count, const Vector3D *vertices);


#endif //BOUNDING_H
