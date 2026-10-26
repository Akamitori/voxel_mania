#ifndef VOXEL_MANIA_QUATERNION_H
#define VOXEL_MANIA_QUATERNION_H

#include "export.h"

struct Vector3D;
struct Matrix3D;


struct EXPORTED Quaternion {
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

EXPORTED Quaternion operator*(const Quaternion &q1, const Quaternion &q2);

EXPORTED Quaternion operator-(const Quaternion &q1);

EXPORTED float dot(const Quaternion &q1, const Quaternion &q2);

EXPORTED Quaternion normalize(const Quaternion &q1);

EXPORTED Vector3D Transform_Vector_With_Quaternion(const Vector3D &v, const Quaternion &q);


#endif //VOXEL_MANIA_QUATERNION_H
