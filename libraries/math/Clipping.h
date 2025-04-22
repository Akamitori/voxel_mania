#ifndef CLIPPING_H
#define CLIPPING_H

#include <cstdint>

#include "export.h"
#include "Plane.h"


struct Matrix4D;
struct OBB;
struct AABB;

constexpr int32_t k_max_polyhedron_vertex_count = 28;
constexpr int32_t k_max_polyhedron_face_count = 16;
constexpr int32_t k_max_polyhedron_edge_count = (k_max_polyhedron_face_count - 2) * 3;
constexpr int32_t k_max_polyhedron_face_edge_count = k_max_polyhedron_face_count - 1;


struct EXPORTED Edge {
    uint8_t vertex_index[2];
    uint8_t face_index[2];
};

struct EXPORTED Face {
    uint8_t edge_count;
    uint8_t edge_index[k_max_polyhedron_face_edge_count];
};

struct EXPORTED Polyhedron {
    uint8_t vertex_count;
    uint8_t edge_count;
    uint8_t face_count;

    Vector3D vertex_points[k_max_polyhedron_vertex_count];
    Edge edge[k_max_polyhedron_edge_count];
    Face face[k_max_polyhedron_face_count];
    Plane plane[k_max_polyhedron_face_count];
};

EXPORTED int clip_polygon(int vertex_count, const Vector3D *vertices, const Plane &plane, float *location,Vector3D *result);

EXPORTED bool clip_polyhedron(const Polyhedron *polyhedron, const Plane *plane, Polyhedron *result);

EXPORTED void build_frustum_polyhedron(const Matrix4D &m_cam, float g, float s, float near, float far, Polyhedron *polyhedron);

EXPORTED bool sphere_visible(int32_t plane_count, const Plane *planeArray, const Vector3D &center_point, float radius);

EXPORTED bool oriented_box_visible(int32_t plane_count, const Plane *plane_array, const OBB &box);

EXPORTED bool AxisAlignedBoxVisible(int32_t plane_count, const Plane *plane_array, const AABB &box);

#endif //CLIPPING_H
