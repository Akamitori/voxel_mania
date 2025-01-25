#ifndef BOUNDING_H
#define BOUNDING_H

#include "export.h"

#include <cstdint>
#include "Vector3D.h"

struct Vector3D;
struct Sphere;
struct Plane;

struct EXPORTED AABB {
    Vector3D center;
    Vector3D size;
};

struct EXPORTED OBB {
    Vector3D center;
    Vector3D size;
    Vector3D axis[3];
};

struct EXPORTED Sphere {
    Vector3D center;
    float radius;
};


EXPORTED Sphere calculate_bounding_sphere(int32_t vertex_count, const Vector3D *vertices);

float calculate_diameter(int32_t vertex_count, const Vector3D *vertices, int32_t *v_a_index, int32_t *v_b_index);

EXPORTED AABB calculate_axis_aligned_bounding_box(int32_t vertex_count, const Vector3D *vertices);

Vector3D min_per_component(const Vector3D &v1, const Vector3D &v2);

Vector3D max_per_component(const Vector3D &v1, const Vector3D &v2);

Vector3D make_perpendicular_vector(const Vector3D &v);

EXPORTED OBB calculate_oriented_bounding_box(int32_t vertex_count, const Vector3D *vertices);

void calculate_secondary_diameter(int32_t vertex_count, const Vector3D *vertices, const Vector3D &axis,
                                  int32_t *v_a_index, int32_t *v_b_index);

void find_extremal_vertices(int32_t vertex_count, const Vector3D *vertices, const Plane &plane, int32_t *v_e_index,
                            int32_t *v_f_index);

void get_primary_box_directions(int32_t vertex_count, const Vector3D *vertices, int32_t v_a_index, int32_t v_b_index,
                                Vector3D *directions);

void get_secondary_box_directions(int32_t vertex_count, const Vector3D *vertices, const Vector3D &axis,
                                  int32_t v_a_index, int32_t v_b_index, Vector3D *directions);


#endif //BOUNDING_H
