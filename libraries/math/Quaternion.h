#ifndef VOXEL_MANIA_QUATERNION_H
#define VOXEL_MANIA_QUATERNION_H

#include "linearalgebra_export.h"

struct Vector3D;
struct Matrix3D;


struct LINEARALGEBRA_EXPORT Quaternion {
    float x{};
    float y{};
    float z{};
    float w{};

    Quaternion();

    Quaternion(float a, float b, float c, float s);

    Quaternion(const Vector3D &v, float s);

    [[nodiscard]] const Vector3D &GetVectorPart() const;

    [[nodiscard]] Matrix3D GetRotationMatrix() const;

    void SetRotationMatrix(const Matrix3D &m);
};

LINEARALGEBRA_EXPORT Quaternion operator*(const Quaternion &q1, const Quaternion &q2);

LINEARALGEBRA_EXPORT Quaternion operator-(const Quaternion &q1);

LINEARALGEBRA_EXPORT float dot(const Quaternion &q1, const Quaternion &q2);

LINEARALGEBRA_EXPORT Quaternion normalize(const Quaternion &q1);

LINEARALGEBRA_EXPORT Vector3D Transform_Vector_With_Quaternion(const Vector3D &v, const Quaternion &q);


#endif //VOXEL_MANIA_QUATERNION_H
