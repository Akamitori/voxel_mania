
#include "Clipping.h"
#include "Transformations.h"
#include "Bounding.h"
#include "Vector3D.h"

#include <algorithm>
#include <Matrix4D.h>
#include <cmath>


constexpr float k_polygon_epsilon = 0.001F;
constexpr float k_polyhedron_epsilon = k_polygon_epsilon;

int clip_polygon(const int vertex_count, const Vector3D *vertices, const Plane &plane, float *location,
                 Vector3D *result) {
    int positive_count = 0;
    int negative_count = 0;

    for (int a = 0; a < vertex_count; a++) {
        const float d = dot_point(plane, vertices[a]);
        location[a] = d;

        if (d > k_polygon_epsilon) positive_count++;
        else if (d < -k_polygon_epsilon) negative_count++;
    }

    // no vertices on the negative side of the plane. copy the original polygon to the result.
    if (negative_count == 0) {
        for (int a = 0; a < vertex_count; a++) {
            result[a] = vertices[a];
        }
        return vertex_count;
    }

    // no vertices on the positive side of the plane
    if (positive_count == 0) {
        return 0;
    }

    int result_count = 0;
    const Vector3D *p1 = &vertices[vertex_count - 1];
    float d1 = location[vertex_count - 1];

    for (int index = 0; index < vertex_count; index++) {
        const Vector3D *p2 = &vertices[index];

        // current vertex is on the negative side
        if (const float d2 = location[index]; d2 < -k_polygon_epsilon) {
            // previous is on the positive side of the plane
            if (d1 > k_polygon_epsilon) {
                const float t = d1 / (d1 - d2);
                result[result_count++] = *p1 * (1.0F - t) + *p2 * t;
            }
        } else {
            // current vertex is positive side of the plane or in plane
            if (d2 > k_polygon_epsilon && d1 < k_polygon_epsilon) {
                // current vertex on positive side and preceding vertex on the negative side
                const float t = d2 / (d2 - d1);
                result[result_count++] = *p2 * (1.0F - t) + *p1 * t;
            }
            result[result_count++] = *p2;

            p1 = p2;
            d1 = d2;
        }
    }

    return result_count;
}

bool clip_polyhedron(const Polyhedron *polyhedron, const Plane &plane, Polyhedron *result) {
    float vertex_location[k_max_polyhedron_vertex_count];
    uint8_t vertex_code[k_max_polyhedron_vertex_count];
    uint8_t edge_code[k_max_polyhedron_edge_count];
    uint8_t vertex_remap[k_max_polyhedron_vertex_count];
    uint8_t edge_remap[k_max_polyhedron_edge_count];
    uint8_t face_remap[k_max_polyhedron_face_count];
    uint8_t plane_edge_table[k_max_polyhedron_face_edge_count];
    uint8_t min_code = 6;
    uint8_t max_code = 0;

    // Classify vertices.
    const uint32_t vertex_count = polyhedron->vertex_count;
    for (uint32_t a = 0; a < vertex_count; a++) {
        vertex_remap[a] = 0xFF;
        const float d = dot_point(plane, polyhedron->vertex_points[a]);
        vertex_location[a] = d;

        const uint8_t code = (d > -k_polyhedron_epsilon) + (d > k_polyhedron_epsilon) * 2;
        if (min_code > code) {
            min_code = code;
        }
        if (max_code < code) {
            max_code = code;
        }
        vertex_code[a] = code;
    }

    if (min_code != 0) {
        *result = *polyhedron; // No vertices on negative side of clip plane.
        return (true);
    }

    if (max_code <= 1) return (false); // No vertices on positive side of clip plane.

    // Classify edges.
    const uint32_t edge_count = polyhedron->edge_count;
    for (uint32_t a = 0; a < edge_count; a++) {
        edge_remap[a] = 0xFF;
        const Edge *edge = &polyhedron->edge[a];
        edge_code[a] = vertex_code[edge->vertex_index[0]] + vertex_code[edge->vertex_index[1]];
    }

    // Determine which faces will be in result.
    uint32_t result_face_count = 0;
    const uint32_t face_count = polyhedron->face_count;
    for (uint32_t a = 0; a < face_count; a++) {
        face_remap[a] = 0xFF;
        const Face *face = &polyhedron->face[a];
        const uint32_t face_edge_count = face->edge_count;
        for (uint32_t b = 0; b < face_edge_count; b++) {
            if (edge_code[face->edge_index[b]] >= 3) {
                // Face has a vertex on the positive side of the plane.
                result->plane[result_face_count] = polyhedron->plane[a];
                face_remap[a] = result_face_count++;
                break;
            }
        }
    }
    // determine the result edges
    uint32_t result_vertex_count = 0, result_edge_count = 0;
    for (uint32_t a = 0; a < edge_count; a++) {
        if (edge_code[a] >= 2) {
            // The edge is not completely clipped away.
            const Edge *edge = &polyhedron->edge[a];
            Edge *result_edge = &result->edge[result_edge_count];
            edge_remap[a] = result_edge_count++;

            result_edge->face_index[0] = face_remap[edge->face_index[0]];
            result_edge->face_index[1] = face_remap[edge->face_index[1]];

            // Loop over both vertices of edge.
            for (int i = 0; i < 2; i++) {
                if (const uint8_t vertex_index = edge->vertex_index[i]; vertex_code[vertex_index] != 0) {
                    // This vertex on positive side of plane or in plane.
                    uint8_t remapped_vertex_index = vertex_remap[vertex_index];
                    if (remapped_vertex_index == 0xFF) {
                        remapped_vertex_index = result_vertex_count++;
                        vertex_remap[vertex_index] = remapped_vertex_index;
                        result->vertex_points[remapped_vertex_index] = polyhedron->vertex_points[vertex_index];
                    }

                    result_edge->vertex_index[i] = remapped_vertex_index;
                } else {
                    // This vertex on negative side, and other vertex on positive side.
                    const uint8_t other_vertex_index = edge->vertex_index[1 - i];
                    const Vector3D &p1 = polyhedron->vertex_points[vertex_index];
                    const Vector3D &p2 = polyhedron->vertex_points[other_vertex_index];
                    const float d1 = vertex_location[vertex_index];
                    const float d2 = vertex_location[other_vertex_index];
                    const float t = d2 / (d2 - d1);
                    result->vertex_points[result_vertex_count] = p2 * (1.0F - t) + p1 * t;
                    result_edge->vertex_index[i] = result_vertex_count++;
                }
            }
        }
    }

    uint32_t plane_edge_count = 0;
    for (uint32_t a = 0; a < face_count; a++) {
        if (const uint8_t remapped_face_index = face_remap[a]; remapped_face_index != 0xFF) {
            // The face is not completely clipped away.
            Edge *new_edge = nullptr;
            uint8_t new_edge_index = 0xFF;

            const Face *face = &polyhedron->face[a];
            const uint32_t face_edge_count = face->edge_count;
            Face *result_face = &result->face[remapped_face_index];
            uint32_t result_face_edge_count = 0;

            for (uint32_t b = 0; b < face_edge_count; b++) // Loop over face's original edges.
            {
                const uint8_t edge_index = face->edge_index[b];
                if (const int32_t code = edge_code[edge_index]; code & 1) {
                    // One endpoint on negative side of plane, and other either
                    // on positive side (code == 3) or in plane (code == 1).
                    if (!new_edge) {
                        // At this point, we know we need a new edge.
                        new_edge_index = result_edge_count;
                        new_edge = &result->edge[result_edge_count];
                        plane_edge_table[plane_edge_count++] = result_edge_count++;
                        *new_edge = Edge{{0xFF, 0xFF}, {remapped_face_index, 0xFF}};
                    }

                    const Edge *edge = &polyhedron->edge[edge_index];
                    const bool ccw = (edge->face_index[0] == a);
                    const bool insert_edge = ccw ^ (vertex_code[edge->vertex_index[0]] == 0);

                    if (code == 3) // Original edge has been clipped.
                    {
                        const uint8_t remapped_edge_index = edge_remap[edge_index];
                        result_face->edge_index[result_face_edge_count++] = remapped_edge_index;
                        const Edge *resultEdge = &result->edge[remapped_edge_index];
                        if (insert_edge) {
                            new_edge->vertex_index[0] = resultEdge->vertex_index[ccw];
                            result_face->edge_index[result_face_edge_count++] = new_edge_index;
                        } else {
                            new_edge->vertex_index[1] = resultEdge->vertex_index[!ccw];
                        }
                    } else // Original edge has been deleted, code == 1.
                    {
                        if (insert_edge) {
                            new_edge->vertex_index[0] = vertex_remap[edge->vertex_index[!ccw]];
                            result_face->edge_index[result_face_edge_count++] = new_edge_index;
                        } else {
                            new_edge->vertex_index[1] = vertex_remap[edge->vertex_index[ccw]];
                        }
                    }
                } else if (code != 0) {
                    // Neither endpoint is on the negative side of the clipping plane.
                    const uint8_t remapped_edge_index = edge_remap[edge_index];
                    result_face->edge_index[result_face_edge_count++] = remapped_edge_index;
                    if (code == 2) plane_edge_table[plane_edge_count++] = remapped_edge_index;
                }
            }


            if (new_edge && std::max(new_edge->vertex_index[0], new_edge->vertex_index[1]) == 0xFF) {
                // The input polyhedron was invalid.
                *result = *polyhedron;
                return (true);
            }

            result_face->edge_count = result_face_edge_count;
        }
    }

    if (plane_edge_count > 2) {
        result->plane[result_face_count] = plane;
        Face *resultFace = &result->face[result_face_count];
        resultFace->edge_count = plane_edge_count;

        for (uint32_t a = 0; a < plane_edge_count; a++) {
            const uint8_t edge_index = plane_edge_table[a];
            resultFace->edge_index[a] = edge_index;

            Edge *resultEdge = &result->edge[edge_index];
            const uint8_t k = (resultEdge->face_index[1] == 0xFF);
            resultEdge->face_index[k] = result_face_count;
        }

        result_face_count++;
    }

    result->vertex_count = result_vertex_count;
    result->edge_count = result_edge_count;
    result->face_count = result_face_count;
    return (true);
}


/// @param m_cam the object space to world space tranformation for the camera
/// @param g projection distance
/// @param s aspect ratio
/// @remark this method assumes direct x conventions, hence we need to account for that
void build_frustum_polyhedron(const Matrix4D &m_cam, float g, float s, const float near,
                              const float far, Polyhedron *polyhedron) {
    polyhedron->vertex_count = 8;
    polyhedron->edge_count = 12;
    polyhedron->face_count = 6;
    
    // generate vertices for the near side
    float y = near / g;
    float x = y * s;

    polyhedron->vertex_points[0] = transform_point(m_cam, Vector3D{x, y, near});
    polyhedron->vertex_points[1] = transform_point(m_cam, Vector3D{x, -y, near});
    polyhedron->vertex_points[2] = transform_point(m_cam, Vector3D{-x, -y, near});
    polyhedron->vertex_points[3] = transform_point(m_cam, Vector3D{-x, y, near});

    y = far / g;
    x = y * s;
    polyhedron->vertex_points[4] = transform_point(m_cam, Vector3D{x, y, far});
    polyhedron->vertex_points[5] = transform_point(m_cam, Vector3D{x, -y, far});
    polyhedron->vertex_points[6] = transform_point(m_cam, Vector3D{-x, -y, far});
    polyhedron->vertex_points[7] = transform_point(m_cam, Vector3D{-x, y, far});

    // generate lateral planes
    const Matrix4D inverse_m_cam = inverse(m_cam);
    const float mx = 1.f / std::sqrt(g * g + s * s);
    const float my = 1.f / std::sqrt(g * g + 1.f);
    polyhedron->plane[0] = transform_plane(inverse_m_cam, Plane{-g * mx, 0, s * mx, 0.f});
    polyhedron->plane[1] = transform_plane(inverse_m_cam, Plane{0, g * my, my, 0.f});
    polyhedron->plane[2] = transform_plane(inverse_m_cam, Plane{g * mx, 0, s * mx, 0.f});
    polyhedron->plane[3] = transform_plane(inverse_m_cam, Plane{0, -g * my, my, 0.f});

    // generate near and far planes;
    Vector3D m_cam_2{m_cam[2].x, m_cam[2].y, m_cam[2].z};
    Vector3D m_cam_3{m_cam[3].x, m_cam[3].y, m_cam[3].z};
    const float d = dot(m_cam_2,m_cam_3);
    polyhedron->plane[4] = {m_cam_2, -(d + near)};
    polyhedron->plane[5] = {-m_cam_2, d + far};

    // generate all edges and lateral faces
    Edge *edge = polyhedron->edge;
    Face *face = polyhedron->face;
    for (int32_t i = 0; i < 4; i++, face++, edge++) {
        edge[0].vertex_index[0] = i;
        edge[0].vertex_index[1] = i + 4;
        edge[0].face_index[0] = i;
        edge[0].face_index[1] = (i - 1) & 3;

        edge[4].vertex_index[0] = i;
        edge[4].vertex_index[1] = (i + 1) & 3;
        edge[4].face_index[0] = 4;
        edge[4].face_index[1] = i;

        edge[8].vertex_index[0] = ((i + 1) & 3) + 4;
        edge[8].vertex_index[1] = i + 4;
        edge[8].face_index[0] = 5;
        edge[8].face_index[1] = i;

        face->edge_count = 4;
        face->edge_index[0] = i;
        face->edge_index[1] = (i + 1) & 3;
        face->edge_index[2] = (i + 4);
        face->edge_index[3] = (i + 8);
    }

    // generate near and far face
    face[0].edge_count = face[1].edge_count = 4;
    face[0].edge_index[0] = 4;
    face[0].edge_index[1] = 5;
    face[0].edge_index[2] = 6;
    face[0].edge_index[3] = 7;

    face[1].edge_index[0] = 8;
    face[1].edge_index[1] = 9;
    face[1].edge_index[2] = 10;
    face[1].edge_index[3] = 11;
}

bool sphere_visible(int32_t plane_count, const Plane *planeArray, const Vector3D &center_point, float radius) {
    float negative_radius = -radius;

    for (int32_t i = 0; i < plane_count; i++) {
        if (dot_point(planeArray[i], center_point) <= negative_radius) {
            return false;
        }
    }

    return true;
}

bool oriented_box_visible(int32_t plane_count, const Plane *plane_array, const OBB &box) {
    for (int32_t i = 0; i < plane_count; i++) {
        const Plane &g = plane_array[i];
        float rg = std::fabs(dot_vector(g, box.axis[0]) * box.size.x) +
                   std::fabs(dot_vector(g, box.axis[1]) * box.size.y) +
                   std::fabs(dot_vector(g, box.axis[2]) * box.size.z);

        if (dot_point(g, box.center) <= -rg) return false;
    }
    return true;
}

bool AxisAlignedBoxVisible(int32_t plane_count, const Plane *plane_array, const AABB &box) {
    for (int32_t i = 0; i < plane_count; i++) {
        const Plane &g = plane_array[i];
        float rg = std::fabs(g.normal.x * box.size.x) +
                   std::fabs(g.normal.y * box.size.y) +
                   std::fabs(g.normal.z * box.size.z);
        if (dot_point(g, box.center) <= -rg) return false;
    }
    return true;
}
