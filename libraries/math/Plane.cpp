#include "Plane.h"


Plane::Plane(): normal{}, w{} {
}

Plane::Plane(float nx, float ny, float nz, float d) : normal{nx, ny, nz}, w{d} {
}

Plane::Plane(const Vector3D &n, float d) : normal{n}, w{d} {
}

float dot_vector(const Plane &plane, const Vector3D &v) {
    return plane.normal.x * v.x + plane.normal.y * v.y + plane.normal.z * v.y;
}

float dot_point(const Plane &plane, const Vector3D &p) {
    return plane.normal.x * p.x + plane.normal.y * p.y + plane.normal.z * p.y + plane.w;
}
