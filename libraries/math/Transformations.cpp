#include "Transformations.h"

#include "Matrix4D.h"
#include "Matrix3D.h"
#include "Vector3D.h"
#include <cmath>
#include <Trigonometry.h>

Matrix3D rotation_x_matrix3D(const float t){
  float c=cos(t);
  float s= sin(t);
  
  return {
      1.f, 0.f, 0.f,
      0.f, c, -s,
      0.f, s, c
  };
}
Matrix4D rotation_x_matrix4D(const float t){
  float c=cos(t);
  float s= sin(t);
  
    return {
        1.f, 0.f, 0.f, 0.f,
        0.f, c, -s, 0.f,
        0.f, s, c, 0.f,
        0.f, 0.f, 0.f, 1.f,
    };
}

Matrix3D rotation_y_matrix3D(const float t){
    float c=cos(t);
    float s= sin(t);
    
    return {
        c, 0.f, s,
        0.f, 1.f, 0.f,
        -s, 0.f, c
    };
  
}
Matrix4D rotation_y_matrix4D(const float t){
    float c=cos(t);
    float s= sin(t);
    return {
        c, 0.f, s, 0.f,
        0.f, 1.f, 0.f, 0.f,
        -s, 0.f, c, 0.f,
        0.f , 0.f, 0.f, 1.f
    };
}

Matrix3D rotation_z_matrix3D(const float t){
    float c=cos(t);
    float s= sin(t);
    
    return {
        c, -s, 0.f,
        s, c, 0.f,
        0.f, 0.f, 1.f
    };
}
Matrix4D rotation_z_matrix4D(const float t){
    float c=cos(t);
    float s= sin(t);
    
    return {
        c, -s, 0.f, 0.f,
        s, c, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 0.f, 1.f
    };
}

Matrix3D scale_matrix3D(const Matrix3D &m, const Vector3D& v){
    Matrix3D result{};
    result[0] = m[0] * v.x;
    result[1] = m[1] * v.y;
    result[2] = m[2] * v.z;
    return result;
}

Matrix4D scale_matrix4D(const Matrix4D &m, const Vector3D& v){
    Matrix4D result{};
    result[0] = m[0] * v.x;
    result[1] = m[1] * v.y;
    result[2] = m[2] * v.z;
    result[3] = m[3];
    return result;
}

Matrix4D translate(const Matrix4D &m, const Vector3D& v){
    Matrix4D result{m};
    result[3] = m[0] * v.x + m[1] * v.y + m[2] * v.z + m[3];
    return result;
}

Matrix4D look_at(const Vector3D& eye, const Vector3D& center, const Vector3D& up){
    const Vector3D f(normalize(center - eye));
    const Vector3D s(normalize(cross(f, up)));
    const Vector3D u(cross(s, f));

    Matrix4D result(1);
    result[0].x = s.x;
    result[1].x = s.y;
    result[2].x = s.z;
    result[0].y = u.x;
    result[1].y = u.y;
    result[2].y = u.z;
    result[0].z = -f.x;
    result[1].z = -f.y;
    result[2].z = -f.z;
    result[3].x = -dot(s, eye);
    result[3].y = -dot(u, eye);
    result[3].z= dot(f, eye);
    return result;
}