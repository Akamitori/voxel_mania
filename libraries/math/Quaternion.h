#ifndef VOXEL_MANIA_QUATERNION_H
#define VOXEL_MANIA_QUATERNION_H

#include "export.h"
#include "Vector3D.h"

class Vector3D;
class Matrix3D; 


struct EXPORTED Quaternion {
    float x, y, z, w;

    Quaternion();
    
    Quaternion(float a, float b, float c, float s);
    
    Quaternion(const Vector3D &v, float s);
    
    const Vector3D& GetVectorPart(void) const;
    
    Matrix3D GetRotationMatrix(void);
    
    void SetRotationMatrix(const Matrix3D& m);
    
};

EXPORTED Quaternion operator*(const Quaternion& q1,const Quaternion& q2);
EXPORTED Quaternion operator-(const Quaternion& q1);
EXPORTED float dot(const Quaternion& q1,const Quaternion& q2);
EXPORTED Quaternion normalize(const Quaternion& q1);
EXPORTED Vector3D Transform(const Vector3D&v, const Quaternion& q);




#endif //VOXEL_MANIA_QUATERNION_H
