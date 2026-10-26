#include "Quaternion.h"

#include <cmath>

#include "Matrix3D.h"
#include "Vector3D.h"


Quaternion::Quaternion() = default;

Quaternion::Quaternion(float a, float b, float c, float s)
    : x(a),
      y(b),
      z(c),
      w(s) {
}

Quaternion::Quaternion(const Vector3D &v, float s)
    : x(v.x),
      y(v.y),
      z(v.z),
      w(s) {
}

const Vector3D &Quaternion::GetVectorPart() const {
    return reinterpret_cast<const Vector3D &>(x);
}

Matrix3D Quaternion::GetRotationMatrix() const {
    const float x2 = x * x;
    const float y2 = y * y;
    const float z2 = z * z;

    const float xy = x * y;
    const float xz = x * z;
    const float yz = y * z;

    const float wx = w * x;
    const float wy = w * y;
    const float wz = w * z;

    return Matrix3D{
        1.0F - 2.0F * (y2 + z2), 2.0F * (xy - wz), 2.0F * (xz + wy),
        2.0F * (xy + wz), 1.0F - 2.0F * (x2 + z2), 2.0F * (yz - wx),
        2.0F * (xz - wy), 2.0F * (yz + wx), 1.0F - 2.0F * (x2 + y2),
    };
}

void Quaternion::SetRotationMatrix(const Matrix3D &m) {
    const Vector3D &column_0 = m[0];
    const Vector3D &column_1 = m[1];
    const Vector3D &column_2 = m[2];

    const float m00 = column_0.x;
    const float m11 = column_1.y;
    const float m22 = column_2.z;

    const float m10 = column_0.y;
    const float m20 = column_0.z;

    const float m21 = column_1.z;
    const float m01 = column_1.x;

    const float m02 = column_2.x;
    const float m12 = column_2.y;


    const float sum = m00 + m11 + m22;
    if (sum > 0.0F) {
        w = sqrt(sum + 1.0F) * 0.5F;
        const float f = 0.25F / w;


        x = (m21 - m12) * f;
        y = (m02 - m20) * f;
        z = (m10 - m01) * f;
    } else if (m00 > m11 && m00 > m22) {
        x = sqrt(m00 - m11 - m22 + 1.0F) * 0.5f;
        const float f = 0.25F / x;

        y = (m10 + m01) * f;
        z = (m02 + m20) * f;
        w = (m21 - m12) * f;
    } else if (m11 > m22) {
        y = sqrt(m11 - m00 - m22 + 1.0F) * 0.5F;
        const float f = 0.25F / y;

        x = (m10 + m01) * f;
        z = (m21 + m12) * f;
        w = (m02 - m20) * f;
    } else {
        z = sqrt(m22 - m00 - m11 + 1.0F) * 0.5F;
        const float f = 0.25F / z;

        x = (m02 + m20) * f;
        y = (m21 + m12) * f;
        w = (m10 - m01) * f;
    }
}

Quaternion operator*(const Quaternion &q1, const Quaternion &q2) {
    return
            {
                q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y,
                q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x,
                q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w,
                q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z

            };
}

Quaternion operator-(const Quaternion &q1) {
    return {
        -q1.x,
        -q1.y,
        -q1.z,
        -q1.w
    };
}

float dot(const Quaternion &q1, const Quaternion &q2) {
    return q1.x * q2.x +
           q1.y * q2.y +
           q1.z * q2.z +
           q1.w * q2.w;
}

Quaternion normalize(const Quaternion &q1) {
    const float len = 1.0f / sqrt(dot(q1, q1));
    return {
        q1.x * len,
        q1.y * len,
        q1.z * len,
        q1.w * len,
    };
}

Vector3D Transform_Vector_With_Quaternion(const Vector3D &v, const Quaternion &q) {
    const Vector3D &b = q.GetVectorPart();
    const float b2 = b.x * b.x + b.y * b.y + b.z * b.z;

    return
            v * (q.w * q.w - b2) +
            b * dot(v, b) * 2.0F +
            cross(b, v) * (q.w * 2.0F);
}
