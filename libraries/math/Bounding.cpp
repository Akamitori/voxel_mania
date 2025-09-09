#include <cstdint>
#include <cmath>

#include "Bounding.h"

#include <algorithm>
#include <cfloat>
#include <Plane.h>
#include <Vector2D.h>

#include "Vector3D.h"


Sphere calculate_bounding_sphere(const int32_t vertex_count, const Vector3D *vertices) {
    int32_t a, b;

    const float d2 = calculate_diameter(vertex_count, vertices, &a, &b);
    Vector3D center_p = (vertices[a] + vertices[b]) * 0.5f;
    float radius = std::sqrt(d2) * 0.5f;

    for (int32_t i = 0; i < vertex_count; i++) {
        Vector3D pv = vertices[i] - center_p;
        const float m2 = magnitude_squared(pv);
        if (m2 > radius * radius) {
            Vector3D q_p = center_p - (pv * (radius / std::sqrt(m2)));
            center_p = (q_p + vertices[i]) * 0.5f;
            radius = magnitude(q_p - center_p);
        }
    }

    return Sphere{.center = center_p, .radius = radius};
}

float calculate_diameter(const int32_t vertex_count, const Vector3D *vertices, int32_t *v_a_index, int32_t *v_b_index) {
    constexpr int32_t k_direction_count = 13;
    constexpr Vector3D direction[k_direction_count] = {
        {1, 0, 0}, {0, 1, 0}, {0, 0, 1},
        {1, 1, 0}, {1, 0, 1}, {0, 1, 1},
        {1, -1, 0}, {1, 0, -1}, {0, 1, -1},
        {1, 1, 1}, {1, -1, 1}, {1, 1, -1},
        {1, -1, -1}
    };

    float d_min[k_direction_count], d_max[k_direction_count];
    int32_t i_min[k_direction_count], i_max[k_direction_count];

    for (int32_t j = 0; j < k_direction_count; j++) {
        const Vector3D u = direction[j];
        d_min[j] = d_max[j] = dot(u, vertices[0]);
        i_min[j] = i_max[j] = 0;

        for (int32_t i = 1; i < vertex_count; i++) {
            if (const float d = dot(u, vertices[i]); d < d_min[j]) {
                d_min[j] = d;
                i_min[j] = i;
            } else if (d > d_max[j]) {
                d_max[j] = d;
                i_max[j] = i;
            }
        }
    }

    float d2 = magnitude_squared(vertices[i_max[0]] - vertices[i_min[0]]);
    int32_t k = 0;

    for (int32_t j = 1; j < k_direction_count; j++) {
        const float m2 = magnitude_squared(vertices[i_max[j]] - vertices[i_min[j]]);
        if (m2 > d2) {
            d2 = m2;
            k = j;
        }
    }


    *v_a_index = i_min[k];
    *v_b_index = i_max[k];

    return d2;
}

AABB calculate_axis_aligned_bounding_box(int32_t vertex_count, const Vector3D *vertices) {
    Vector3D v_min = vertices[0];
    Vector3D v_max = vertices[0];

    for (int32_t i = 1; i < vertex_count; i++) {
        v_min = min_per_component(v_min, vertices[i]);
        v_max = max_per_component(v_max, vertices[i]);
    }

    return {.center = (v_min + v_max) * 0.5f, .size = (v_max - v_min) * 0.5f};
}

Vector3D min_per_component(const Vector3D &v1, const Vector3D &v2) {
    return {std::min(v1.x, v2.x), std::min(v1.y, v2.y), std::min(v1.z, v2.z)};
}

Vector3D max_per_component(const Vector3D &v1, const Vector3D &v2) {
    return {std::max(v1.x, v2.x), std::max(v1.y, v2.y), std::max(v1.z, v2.z)};
}

Vector3D make_perpendicular_vector(const Vector3D &v) {
    const float x = std::fabs(v.x);
    const float y = std::fabs(v.y);
    const float z = std::fabs(v.z);

    if (z < std::min(x, y)) {
        return Vector3D{v.y, -v.x, 0.0f};
    }

    if (y < x) {
        return Vector3D{-v.z, 0.f, v.x};
    }

    return Vector3D{0.f, v.z, -v.y};
}

OBB calculate_oriented_bounding_box(int32_t vertex_count, const Vector3D *vertices) {
    int32_t a_index, b_index;
    Vector3D primary_direction[9];
    Vector3D secondary_direction[5];

    calculate_diameter(vertex_count, vertices, &a_index, &b_index);
    get_primary_box_directions(vertex_count, vertices, a_index, b_index, primary_direction);

    float area = FLT_MAX;
    OBB result{};
    for (int32_t k = 0; k < 9; k++) {
        Vector3D s = normalize(primary_direction[k]);
        calculate_secondary_diameter(vertex_count, vertices, s, &a_index, &b_index);
        get_secondary_box_directions(vertex_count, vertices, s, a_index, b_index, secondary_direction);

        for (int32_t j = 0; j < 5; j++) {
            Vector3D t = normalize(secondary_direction[j]);
            Vector3D u = cross(s, t);
            float s_min = dot(s, vertices[0]), s_max = s_min;
            float t_min = dot(t, vertices[0]), t_max = t_min;
            float u_min = dot(u, vertices[0]), u_max = u_min;

            for (int32_t i = 1; i < vertex_count; i++) {
                float ds = dot(s, vertices[i]);
                float dt = dot(t, vertices[i]);
                float du = dot(u, vertices[i]);

                s_min = std::min(s_min, ds);
                s_max = std::max(s_max, ds);

                t_min = std::min(t_min, dt);
                t_max = std::max(t_max, dt);

                u_min = std::min(u_min, du);
                u_max = std::max(u_max, du);
            }

            const float hx = (s_max - s_min) * 0.5f;
            const float hy = (t_max - t_min) * 0.5f;
            const float hz = (u_max - u_min) * 0.5f;

            const float m = hx * hy + hy * hz + hz * hx;
            if (m < area) {
                result.center = (s * (s_min + s_max) + t * (t_min + t_max) + u * (u_min + u_max)) * 0.5f;
                result.size = {hx, hy, hz};
                result.axis[0] = s;
                result.axis[1] = t;
                result.axis[2] = u;
                area = m;
            }
        }
    }

    return result;
}

void calculate_secondary_diameter(const int32_t vertex_count, const Vector3D *vertices, const Vector3D &axis,
                                  int32_t *v_a_index, int32_t *v_b_index) {
    constexpr int32_t k_direction_count = 4;
    constexpr Vector2D direction[k_direction_count]{
        {1, 0},
        {0, 1},
        {1, 1},
        {1, -1}
    };

    float d_min[k_direction_count], d_max[k_direction_count];
    int32_t i_min[k_direction_count], i_max[k_direction_count];

    const Vector3D x = make_perpendicular_vector(axis);
    const Vector3D y = cross(axis, x);

    for (int32_t j = 0; j < k_direction_count; j++) {
        Vector3D t = x * direction[j].x + y * direction[j].y;
        d_min[j] = d_max[j] = dot(t, vertices[0]);
        i_min[j] = i_max[j] = 0;

        for (int32_t i = 1; i < vertex_count; i++) {
            const float d = dot(t, vertices[i]);

            if (d < d_min[j]) {
                d_min[j] = d;
                i_min[j] = i;
            } else if (d > d_max[j]) {
                d_max[j] = d;
                i_max[j] = i;
            }
        }
    }

    Vector3D dv = vertices[i_max[0]] - vertices[i_min[0]];
    int32_t k = 0;
    float d2 = magnitude_squared(dv - axis * dot(dv, axis));

    for (int32_t j = 1; j < k_direction_count; j++) {
        dv = vertices[i_max[j]] - vertices[i_min[j]];
        const float m2 = magnitude_squared(dv - axis * dot(dv, axis));
        if (m2 > d2) {
            d2 = m2;
            k = j;
        }
    }

    *v_a_index = i_min[k];
    *v_b_index = i_max[k];
}

void find_extremal_vertices(const int32_t vertex_count, const Vector3D *vertices, const Plane &plane,
                            int32_t *v_e_index,
                            int32_t *v_f_index) {
    *v_e_index = 0;
    *v_f_index = 0;

    float d_min = dot_point(plane, vertices[0]);
    float d_max = d_min;

    for (int32_t i = 1; i < vertex_count; i++) {
        const float m = dot_point(plane, vertices[i]);
        if (m < d_min) {
            d_min = m;
            *v_e_index = i;
        } else if (m > d_max) {
            d_max = m;
            *v_f_index = i;
        }
    }
}

void get_primary_box_directions(
    int32_t vertex_count,
    const Vector3D *vertices,
    const int32_t v_a_index,
    const int32_t v_b_index,
    Vector3D *directions
) {
    int32_t c = 0;
    directions[0] = vertices[v_b_index] - vertices[v_a_index];
    float d_max = distance_between_point_and_line(vertices[0], vertices[v_a_index], directions[0]);
    for (int32_t i = 1; i < vertex_count; ++i) {
        const float m = distance_between_point_and_line(vertices[i], vertices[v_a_index], directions[0]);
        if (m > d_max) {
            d_max = m;
            c = i;
        }
    }

    directions[1] = vertices[c] - vertices[v_a_index];
    directions[2] = vertices[c] - vertices[v_b_index];
    Vector3D normal = cross(directions[0], directions[1]);
    Plane plane{normal, -dot(normal, vertices[v_a_index])};

    int32_t e, f;

    find_extremal_vertices(vertex_count, vertices, plane, &e, &f);

    directions[3] = vertices[e] - vertices[v_a_index];
    directions[4] = vertices[e] - vertices[v_b_index];
    directions[5] = vertices[e] - vertices[c];

    directions[6] = vertices[f] - vertices[v_a_index];
    directions[7] = vertices[f] - vertices[v_b_index];
    directions[8] = vertices[f] - vertices[c];
}

void get_secondary_box_directions(
    const int32_t vertex_count,
    const Vector3D *vertices,
    const Vector3D &axis,
    const int32_t v_a_index,
    const int32_t v_b_index,
    Vector3D *directions
) {
    directions[0] = vertices[v_b_index] - vertices[v_a_index];
    Vector3D normal = cross(axis, directions[0]);
    Plane plane{normal, -dot(normal, vertices[v_a_index])};

    int32_t e, f;
    find_extremal_vertices(vertex_count, vertices, plane, &e, &f);

    directions[1] = vertices[e] - vertices[v_a_index];
    directions[2] = vertices[e] - vertices[v_b_index];
    directions[3] = vertices[f] - vertices[v_a_index];
    directions[4] = vertices[f] - vertices[v_b_index];

    for (int32_t j = 0; j < 5; j++) {
        directions[j] -= axis * dot(directions[j], axis);
    }
}
