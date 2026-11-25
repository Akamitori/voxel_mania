#include <array>
#include <exception>

#include <SDL3/SDL_init.h>

#include "InputHandling.h"
#include "cube.h"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>

#include "Trigonometry.h"
#include "Renderer.h"
#include "SDL3/SDL_timer.h"

#include "quad.h"
#include "libraries/Renderer/Camera.h"

#include "Main_Containers.h"
#include "Perlin.h"
#include "AssetLoader.h"
#include "math_ops.h"
#include "Matrix3D.h"
#include "Transformations.h"

// void calculate_frustum_planes_and_check_for_visibility(const Matrix4D &world_space_matrix, AppData &appData,
//                                                        const Matrix4D &triangle_model_view_space,
//                                                        const cube &my_cube) {
//     Vector3D vertex_data[8];
//
//     build_frustum_polyhedron(
//         inverse(appData.camera_matrix),
//         1 / tan(DegreeToRadians(appData.FOV) * 0.5f),
//         static_cast<float>(initial_screen_width) / static_cast<float>(initial_screen_height),
//         appData.z_near, appData.z_far, &appData.frustum_polyhedron);
//
//     for (int i = 0, j = 0; i < 8 && j < 24; i++, j += 3) {
//         auto point = Vector3D{
//             my_cube.vertex_data[j],
//             my_cube.vertex_data[j + 1],
//             my_cube.vertex_data[j + 2]
//         };
//
//         // get the cube to global space
//         vertex_data[i] = transform_point(world_space_matrix, transform_point(triangle_model_view_space, point));
//     }
//
//     // check if the cube is contained in the frustum
//     auto a = calculate_axis_aligned_bounding_box(8, vertex_data);
//     auto result = AxisAlignedBoxVisible(6, appData.frustum_polyhedron.plane, a);
//     std::cout << "Box contained : " << result << std::endl;
// }

// draw this model at this position
//

static float sample_sphere_density(Vector3D &p, float radius) {
    return magnitude(p) - radius;
}

static float sample_cirlce_density(const Vector2D &p, const Vector2D &circle_center, float radius) {
    // express the point in terms of the circles local space
    const Vector2D res{p.x - circle_center.x, p.y - circle_center.y};
    double magnitude = sqrt(res.x * res.x + res.y * res.y);
    return magnitude - radius;
}

// assume no rotation or translation for now
static float sample_plane_density(const Vector3D &p, const Vector3D &normal) {
    return dot(p, normal);
}

struct Ray3D {
    Vector3D origin;
    // normalized
    Vector3D direction;
};


int RegisterCubeMesh3Part(int diffuse_texture_id, int specular_texture_id, int emission_texture_id = -1) {
    return Renderer_RegisterTexturedMesh(
        diffuse_texture_id,
        specular_texture_id,
        emission_texture_id,
        cube::vertex_data_uv_3_part_texture,
        cube::vertices_count_uv,
        cube::vertex_indices_uvs,
        cube::vertex_indices_count_uv
    );
}

int RegisterCubeMesh1Part(int diffuse_texture_id, int specular_texture_id, int emission_texture_id = -1) {
    return Renderer_RegisterTexturedMesh(
        diffuse_texture_id,
        specular_texture_id,
        emission_texture_id,
        cube::vertex_data_uv_1_part_texture,
        cube::vertices_count_uv,
        cube::vertex_indices_uvs,
        cube::vertex_indices_count_uv
    );
}

int RegisterQuadMesh1Part(int diffuse_texture_id, int specular_texture_id, int emission_texture_id = -1) {
    return Renderer_RegisterTexturedMesh(
        diffuse_texture_id,
        specular_texture_id,
        emission_texture_id,
        quad::vertex_data_uv_1_part_texture_single_faced,
        quad::vertices_count_uv_single_faced,
        quad::vertex_indices_uvs_single_faced,
        quad::vertex_indices_count_uv_single_faced
    );
}

int RegisterQuadMesh2Part(int diffuse_texture_id, int specular_texture_id, int emission_texture_id = -1) {
    return Renderer_RegisterTexturedMesh(
        diffuse_texture_id,
        specular_texture_id,
        emission_texture_id,
        quad::vertex_data_uv_1_part_texture,
        quad::vertices_count_uv,
        quad::vertex_indices_uvs,
        quad::vertex_indices_count_uv
    );
}

// int compare_models_descending(const void *p, const void *q) {
//     const mesh_instance m1 = *(const mesh_instance *) p;
//     const mesh_instance m2 = *(const mesh_instance *) q;
//
//     const float distance1 = magnitude_squared(SceneCamera->position - m1.transform.Position);
//     const float distance2 = magnitude_squared(SceneCamera->position - m2.transform.Position);
//
//     return (distance1 < distance2) - (distance1 > distance2);
// }
//
// void sort_objects_based_on_camera_distance(mesh_instance *models, const size_t number) {
//     qsort(models, number, sizeof(mesh_instance), compare_models_descending);
// }

void Create_Scene(const int wood_cube_id, const int crate_cube_id, Vector_mesh_instance *opaque_meshes) {
    // create a scene for shadow testing
    for (int x = 0; x < 50; ++x) {
        for (int y = 0; y < 50; ++y) {
            Vector_mesh_instance_Add(opaque_meshes, {wood_cube_id, {(float) x - 5, (float) y + 2, -1}});
        }
    }

    Vector_mesh_instance_Add(opaque_meshes, {wood_cube_id, {(float) 4, (float) 11, 0}});
    Vector_mesh_instance_Add(opaque_meshes, {wood_cube_id, {(float) 4, (float) 11, 1}});

    Vector_mesh_instance_Add(opaque_meshes, {crate_cube_id, {(float) 4 + 2, (float) 11, 0}});
    Vector_mesh_instance_Add(opaque_meshes, {crate_cube_id, {(float) 4 + 2, (float) 11, 1}});
    Vector_mesh_instance_Add(opaque_meshes, {crate_cube_id, {(float) 4 + 2, (float) 11, 2}});
    Vector_mesh_instance_Add(opaque_meshes, {crate_cube_id, {(float) 4 + 2, (float) 11, 3}});


    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 10; ++j) {
            Vector_mesh_instance_Add(opaque_meshes, {crate_cube_id, {(float) j + 1, (float) 10 + i * 2 + 1, 0}});
            Vector_mesh_instance_Add(opaque_meshes, {crate_cube_id, {(float) j + 1, (float) 10 + i * 2 + 1, 1}});
            Vector_mesh_instance_Add(opaque_meshes, {crate_cube_id, {(float) j + 1, (float) 10 + i * 2 + 1, 2}});
            Vector_mesh_instance_Add(opaque_meshes, {crate_cube_id, {(float) j + 1, (float) 10 + i * 2 + 1, 3}});
        }
    }

    // for (int i = 0; i < 4; ++i) {
    //     for (int j = 0; j < 10; ++j) {
    //         Vector_mesh_instance_Add(opaque_meshes, {crate_cube_id, {(float) j + 20, (float) 30 + i * 2 + 1, 0}});
    //         Vector_mesh_instance_Add(opaque_meshes, {crate_cube_id, {(float) j + 20, (float) 30 + i * 2 + 1, 1}});
    //         Vector_mesh_instance_Add(opaque_meshes, {crate_cube_id, {(float) j + 20, (float) 30 + i * 2 + 1, 2}});
    //         Vector_mesh_instance_Add(opaque_meshes, {crate_cube_id, {(float) j + 20, (float) 30 + i * 2 + 1, 3}});
    //     }
    // }


    // Vector_mesh_instance_Add(opaque_meshes, {crate_cube_id, {(float) 5 - 5, (float) 6, 2}});
    // Vector_mesh_instance_Add(opaque_meshes, {crate_cube_id, {(float) 7 - 5, (float) 4, 0}});
    // Vector_mesh_instance_Add(opaque_meshes, {
    //                              crate_cube_id, {
    //                                  (float) 3 - 5, (float) 3, 0,
    //                                  0, DegreeToRadians(45), DegreeToRadians(45)
    //                              }
    //
    //                          });


    // for (size_t i = 0; i < Vector_mesh_instance_Length(opaque_meshes); ++i) {
    //     const Vector3D v = opaque_meshes->data[i].transform.Position;
    //     printf("(%3f, %3f, %3f)", v.x, v.y, v.z);
    // }


    // const int offset = 40;
    // for (int x = 0; x < 10; ++x) {
    //     for (int y = 0; y < 10; ++y) {
    //         Vector_mesh_instance_Add(opaque_meshes, {wood_cube_id, {(float) offset + x - 5, (float) y + 2, -1}});
    //     }
    // }
}


constexpr size_t samples_per_axis_3D = 64;
constexpr float step = 0.5f;
constexpr float sphere_radius = 5;
constexpr size_t cells_per_axis_3D = samples_per_axis_3D - 1;
const Vector3D grid_origin_3D = Vector3D{.x = step, .y = step, .z = step} * (
                                    -static_cast<float>(cells_per_axis_3D) / 2.0f);
constexpr size_t total_sampling_points = samples_per_axis_3D * samples_per_axis_3D * samples_per_axis_3D;

constexpr size_t samples_per_axis_2D = 128;
constexpr size_t max_line_no = 2 * (samples_per_axis_2D - 1) * (samples_per_axis_2D - 1);
constexpr float step_2D = 0.25f;
constexpr size_t cells_per_axis_2D = samples_per_axis_2D - 1;
constexpr float circle_radius = 1;
constexpr float offset_to_center_2D = -static_cast<float>(cells_per_axis_2D) / 2.0f;
constexpr float far_plane_2d = 30;
constexpr Vector3D grid_origin_2D = Vector3D{
    .x = step_2D * offset_to_center_2D, .y = far_plane_2d, .z = step_2D * offset_to_center_2D
};
constexpr size_t total_sampling_points_2D = samples_per_axis_2D * samples_per_axis_2D;
constexpr size_t cell_count_per_axis = samples_per_axis_2D - 1;


const size_t no_edge = -1;

bool get_line_point(size_t edge_corners[4][2],
                    const size_t edge_index,
                    const float cell_samples[4],
                    const Vector3D cell_corners[4],
                    Vector3D &result) {
    if (edge_index == no_edge) {
        return false;
    }

    const size_t e_0_corner_0_index = edge_corners[edge_index][0];
    const size_t e_0_corner_1_index = edge_corners[edge_index][1];

    const float edge_0_d0 = cell_samples[e_0_corner_0_index];
    const float edge_0_d1 = cell_samples[e_0_corner_1_index];
    const float interpol_factor_0 = edge_0_d0 / (edge_0_d0 - edge_0_d1);

    const Vector3D edge_0_corner_0 = cell_corners[e_0_corner_0_index];
    const Vector3D edge_0_corner_1 = cell_corners[e_0_corner_1_index];
    const Vector3D vertice_0 = edge_0_corner_0 * (1 - interpol_factor_0)
                               + edge_0_corner_1 * interpol_factor_0;
    result = vertice_0;
    return true;
}

Vector3D *Calc_Circle(size_t samples = 64, float y = 10, float radius = 1) {
    //point(θ) = (cx + r·cos θ,   y,   cz + r·sin θ)

    float step = TWOPI / samples;
    Vector3D *points = (Vector3D *) malloc(sizeof(Vector3D) * samples);
    for (size_t i = 0; i < samples; ++i) {
        const float x = radius * cos(step * i);
        const float z = radius * sin(step * i);
        points[i] = Vector3D{x, y, z};
    }

    return points;
}


// get's a pixel. provides us with a point in front of the camera(at camera-space z = 1.)
// that's part of the inf ray that goes through that particular pixel
Ray3D pixel_to_ray_in_world_space(Camera *camera, int width, int height, float x_pixel, float y_pixel, float FOV) {
    const float x_ndc = x_pixel * (2.0f / width) - 1.0f;
    const float y_ndc = 1.0f - y_pixel * 2.0f / height;

    const float g = 1 / tan(DegreeToRadians(FOV) * 0.5f);
    const float a = width * 1.0 / height;

    //we do some equation magic and pick z=1 since we dont care about depth for the ray
    // it's a ray after all!
    const Vector3D p_camera_space = Vector3D(x_ndc * a / g, -y_ndc / g, 1);

    const Vector3D p_world_space = transform_point(camera->Camera_Matrix, p_camera_space);

    const Vector3D ray_origin = camera->position;
    const Vector3D ray_dir = normalize(p_world_space - ray_origin);

    return Ray3D{ray_origin, ray_dir};
}

static Vector3D find_ray_and_plane_intersection(const Ray3D &ray, const Vector3D plane_normal,
                                                const Vector3D plane_point) {
    const float plane_h = dot(plane_point, plane_normal);
    const float ray_t = (plane_h - dot(ray.origin, plane_normal)) / dot(
                            ray.direction, plane_normal);

    const Vector3D intersect_at = ray.origin + ray.direction * ray_t;
    return intersect_at;
}

void sample_contours(float *samples, Line2D *lines, size_t &line_count) {
    float cell_samples[4];
    // which edges provide a point
    size_t edge_cases[16][4]{
        {no_edge, no_edge, no_edge, no_edge},
        {0, 3, no_edge, no_edge},
        {0, 1, no_edge, no_edge},
        {1, 3, no_edge, no_edge},

        {1, 2, no_edge, no_edge},
        {0, 1, 2, 3},
        {0, 2, no_edge, no_edge},
        {2, 3, no_edge, no_edge},

        {2, 3, no_edge, no_edge},
        {0, 2, no_edge, no_edge},
        {0, 3, 1, 2},
        {1, 2, no_edge, no_edge},

        {1, 3, no_edge, no_edge},
        {0, 1, no_edge, no_edge},
        {0, 3, no_edge, no_edge},
        {no_edge, no_edge, no_edge, no_edge},
    };

    // what are the corners of those edges
    size_t edge_corners[4][2] = {
        // bottom edge
        {0, 1},
        // right edge
        {1, 2},
        // top edge
        {3, 2},
        // left edge
        {0, 3}
    };

    uint8_t case_index;
    for (size_t z = 0; z < cells_per_axis_2D; ++z) {
        for (size_t x = 0; x < cells_per_axis_2D; ++x) {
            const size_t lower_left = z * samples_per_axis_2D + x;
            const size_t lower_right = lower_left + 1;
            const size_t upper_left = lower_left + samples_per_axis_2D;
            const size_t upper_right = upper_left + 1;

            cell_samples[0] = samples[lower_left];
            cell_samples[1] = samples[lower_right];
            cell_samples[2] = samples[upper_right];
            cell_samples[3] = samples[upper_left];

            const bool sample_lower_left = cell_samples[0] > 0;
            const bool sample_lower_right = cell_samples[1] > 0;

            const bool sample_upper_right = cell_samples[2] > 0;
            const bool sample_upper_left = cell_samples[3] > 0;

            case_index = 0;

            case_index |= sample_lower_left;
            case_index |= sample_lower_right << 1;
            case_index |= sample_upper_right << 2;
            case_index |= sample_upper_left << 3;

            if (case_index == 0 || case_index == 15) {
                continue;
            }

            const Vector3D bottom_left_corner = grid_origin_2D + Vector3D{x * step_2D, 0, z * step_2D};

            const Vector3D cell_corners[4] = {
                bottom_left_corner,
                bottom_left_corner + Vector3D{step_2D, 0, 0},
                bottom_left_corner + Vector3D{step_2D, 0, step_2D},
                bottom_left_corner + Vector3D{0, 0, step_2D}
            };

            // which edges are we using
            size_t (*sampling_case)[4] = &edge_cases[case_index];

            // our formulais d(t)= d0+t(d1-d0). solve for t you get the pick
            for (size_t sample_edges = 0; sample_edges < 4; sample_edges += 2) {
                Line2D result{};
                Vector3D point{};
                const size_t edge_index_0 = (*sampling_case)[sample_edges];
                if (!get_line_point(edge_corners, edge_index_0, cell_samples, cell_corners, point)) {
                    break;
                }
                result.point1 = point;

                const size_t edge_index_1 = (*sampling_case)[sample_edges + 1];
                if (!get_line_point(edge_corners, edge_index_1, cell_samples, cell_corners, point)) {
                    break;
                }
                result.point2 = point;
                lines[line_count++] = result;
            }
        }
    }
}

Vector3D *Calculate_Grid_Outline(size_t &grid_point_count) {
    Vector3D *grid_points = (Vector3D *) malloc(samples_per_axis_2D * 4 * sizeof(Vector3D));
    grid_point_count = 0;

    const float x_start = grid_origin_2D.x;
    const float x_end = x_start + step_2D * cells_per_axis_2D;

    const float z_start = grid_origin_2D.z;
    const float z_end = grid_origin_2D.z + step_2D * cells_per_axis_2D;

    const float depth = far_plane_2d + 0.02;

    for (size_t i = 0; i < samples_per_axis_2D; ++i) {
        const float z_vertical = grid_origin_2D.z + step_2D * i;
        Vector3D line_along_x_v1{x_start, depth, z_vertical};
        Vector3D line_along_x_v2{x_end, depth, z_vertical};

        grid_points[grid_point_count++] = line_along_x_v1;
        grid_points[grid_point_count++] = line_along_x_v2;

        //const float height
        const float x_horizontal = grid_origin_2D.x + step_2D * i;
        Vector3D line_along_z_v1 = {x_horizontal, depth, z_start};
        Vector3D line_along_z_v2 = {x_horizontal, depth, z_end};

        grid_points[grid_point_count++] = line_along_z_v1;
        grid_points[grid_point_count++] = line_along_z_v2;
    }
    return grid_points;
}

float union_sdf(float d1, float d2) {
    return math_ops::min(d1, d2);
}

float subtraction_sdf(float d1, float d2) {
    return math_ops::max(d1, -d2);
}


float smin(float a, float b, float k) {
    float h = math_ops::max(k - abs(a - b), 0.0f) / k;
    return math_ops::min(a, b) - h * h * k * (1.0f / 4.0f);
}

float smax(float a, float b, float k) {
    return -smin(-a, -b, k);
}

float union_sdf_smooth(float d1, float d2,float k) {
    return smin(d1, d2,k);
}

float subtraction_sdf_smooth(float d1, float d2, float k) {
    return smax(d1, -d2,k);
}


typedef float (*sdf_operation)(float a, float v);
typedef float (*sdf_operation_smooth)(float a, float v,float k);

int main() {
    float *samples = (float *) malloc(sizeof(float) * total_sampling_points_2D);
    Vector3D *points = (Vector3D *) malloc(sizeof(Vector3D) * total_sampling_points_2D);

    Vector3D *cirlce_points = Calc_Circle(64, -0.01f, 1);

    constexpr int circle_count = 7;
    Vector2D circle_positions[circle_count] = {
        {0, 0},
        {0.5, 0.5},
        {5.5, 5.5},
        {6, 6.1},
        {10, 10},
        {-6, 0},
        {-4.2, 0}
    };

    for (size_t z = 0; z < samples_per_axis_2D; ++z) {
        for (size_t x = 0; x < samples_per_axis_2D; ++x) {
            size_t index = z * samples_per_axis_2D
                           + x;

            Vector3D world_pos = grid_origin_2D + Vector3D{x * step_2D, 0.01, z * step_2D};
            points[index] = world_pos;

            float united_dfs = FLT_MAX;

            for (size_t i = 0; i < circle_count; ++i) {
                const Vector2D &pos = circle_positions[i];
                const float iteration_dfs = sample_cirlce_density(
                    {world_pos.x, world_pos.z}, pos,
                    circle_radius
                );
                united_dfs = math_ops::min(united_dfs, iteration_dfs);
            }
            samples[index] = united_dfs;
        }
    }

    size_t grid_point_count;
    Vector3D *grid_points = Calculate_Grid_Outline(grid_point_count);


    Line2D *contour_lines = (Line2D *) malloc(sizeof(Line2D) * max_line_no);
    size_t contour_line_count = 0;

    sample_contours(samples, contour_lines, contour_line_count);

    printf("sample count is %zu\n", contour_line_count);

    try {
        //parse_scene("data/models/boblampclean.md5mesh");
        //return 0;
        constexpr int game_resolution_width = 1920, game_resolution_height = 1080;
        Renderer_Init(game_resolution_width, game_resolution_height, 45, 0.1, 1024, 8);
        DEBUG_SendPointDataToTheGPU(points, samples, total_sampling_points_2D);
        DEBUG_SendContourDataToTheGPU(contour_lines, contour_line_count);
        DEBUG_SendGridDataToTheGPU(grid_points, grid_point_count);
        DEBUG_SendCircleCursorDataToTheGPU(cirlce_points, 64, {1, 0, 0});

        auto display_properties = SDL_GetCurrentDisplayMode(1);

        int width = display_properties->w;
        int height = display_properties->h;

        const int whiteTextureId = Renderer_RegisterTextureFromPath("data/textures/white_texture.png",
                                                                    {.convert_from_srgb_to_linear_space = true});
        const int woodTextureId = Renderer_RegisterTextureFromPath("data/textures/wood.png",
                                                                   {.convert_from_srgb_to_linear_space = true});
        const int grassTextureId = Renderer_RegisterTextureFromPath("data/textures/grass_block.png",
                                                                    {.convert_from_srgb_to_linear_space = true});
        const int mushroomTextureId = Renderer_RegisterTextureFromPath("data/textures/mushroom_red.png",
                                                                       {.convert_from_srgb_to_linear_space = true});
        const int crate_Texture_diffuse_id = Renderer_RegisterTextureFromPath("data/textures/box_container.png", {
                                                                                  .convert_from_srgb_to_linear_space =
                                                                                  true
                                                                              });
        const int crate_Texture_specular_id = Renderer_RegisterTextureFromPath(
            "data/textures/box_container_specular.png"); // specular stays as is
        const int tranrsparent_window_texture = Renderer_RegisterTextureFromPath(
            "data/textures/blending_transparent_window.png",
            {
                TextureWrapMode::CLAMP_TO_EDGE,
                TextureWrapMode::CLAMP_TO_EDGE,
                true
            }
        );

        // // if our problem is wrapping cubes and data like that
        // // we can probably have the API acknowledge that
        // const int grass_cube_id = RegisterCubeMesh3Part(
        //     grassTextureId, grassTextureId
        // );
        //
        // const int white_cube_id = RegisterCubeMesh3Part(
        //     whiteTextureId, whiteTextureId
        // );
        //
        const int wood_cube_id = RegisterCubeMesh1Part(woodTextureId, woodTextureId);
        //
        const int crate_cube_id = RegisterCubeMesh1Part(crate_Texture_diffuse_id, crate_Texture_specular_id);
        //
        // const int glass_cube_id = RegisterQuadMesh2Part(tranrsparent_window_texture, tranrsparent_window_texture);
        //
        // const int mushroom_cube_id = Renderer_RegisterTextured_Cross_Mesh(
        //     mushroomTextureId,
        //     0.5f
        // );
        //
        //
        const int light_source = Renderer_RegisterUnshadedTexture(
            cube::vertex_data,
            cube::vertices_count,
            cube::vertex_indices,
            cube::indices_count
        );


        //const int cube_id_5=RegisterCubeMesh3Part(cubeId_1);

        // loads a model . skip for now because this is slow as fuck
        //const int back_pack_model = Renderer_Register_Model("data/models/backpack/backpack.obj");
        const int bob = Renderer_Register_Model("data/models/boblampclean.md5mesh");


        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // so we can dock things to windows
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // so our ui can exist outside the window

        ImGui::GetStyle().ScaleAllSizes(1.2f);
        //io.ConfigViewportsNoAutoMerge = true;
        //io.ConfigViewportsNoTaskBarIcon = true;

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsLight();

        // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
        ImGuiStyle &style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        ImGui_ImplSDL3_InitForOpenGL(window, open_gl_context);
        ImGui_ImplOpenGL3_Init();

        Vector3D light_color = {1, 1, 1};


        Vector_mesh_instance *opaque_meshes = Vector_mesh_instance_Create(30);
        Vector_mesh_instance *transparent_models = Vector_mesh_instance_Create(30);

        Create_Scene(wood_cube_id, crate_cube_id, opaque_meshes);

        // Target cube πιο μακριά
        // Vector_mesh_instance_Add(opaque_models, {crate_cube_id, {0, 2, 0}});
        // Vector_mesh_instance_Add(opaque_models, {crate_cube_id, {1, 2, 0}});
        // Vector_mesh_instance_Add(opaque_models, {crate_cube_id, {1, -2, 0}});

        // Vector_mesh_instance_Add(transparent_models, {glass_cube_id, {0, 1.5, 0}});
        // Vector_mesh_instance_Add(transparent_models, {glass_cube_id, {0.5, 1, 0}});

        //         PointLight our_light{
        //             Vector3D{-5, 5, 2}, // Αντίθετη πλευρά από την κάμερα
        // {0.03f, 0.03f, 0.03f},  // ambient - πολύ χαμηλό
        // {0.8f, 0.8f, 0.8f},      // diffuse - δυνατό, τώρα θα φαίνεται σωστά
        // {1.0f, 1.0f, 1.0f},      // specular
        // 0.045f,                   // linear - χαμηλότερο, πιο soft falloff
        // 0.0075f
        //         };


        Vector3D spotLightpos = SceneCamera->position;
        spotLightpos.z += 0.5f;
        SpotLight our_spot_light{
            {5, 5, 3},
            {0, 0, -1},
            {0.1f, 0.1f, 0.1f}, // Καθόλου ambient (ήταν 0.01)
            {0.8f, 0.8f, 0.8f}, // Πολύ πιο δυνατό (ήταν 0.3)
            {0.1f, 0.1f, 0.1f},
            0.09f,
            0.032f,
            (float) cos(DegreeToRadians(12.5f)),
            (float) cos(DegreeToRadians(17.5f)),
        };
        // DirectionalLight our_dir_light{
        //     {0, -1, 0.2f}, // Από πάνω προς τα κάτω
        //     {0.1f, 0.1f, 0.1f}, // Minimal ambient (ήταν 0.1)
        //     {0.15f, 0.15f, 0.15f}, // Πολύ αχνό diffuse (ήταν 0.7)
        //     {0.3f, 0.3f, 0.3f}
        // };

        DirectionalLight our_dir_light{
            //{0.8f, 0.2f, -0.5f}, // direction
            //{0.3f, 0.5f, -0.8f},
            {1.0f, 0.0f, -1.0f}, // pointing straight down

            {0.05f, 0.05f, 0.05f}, // ambient
            {0.8f, 0.75f, 0.7f}, // diffuse - bright enough to see
            {0.5f, 0.5f, 0.5f} // specular - some highlights
        };

        PointLight our_light{
            Vector3D{5, 5, 5}, // κέντρο-ish του scene
            {0.0f, 0.0f, 0.0f},
            {1.0f, 0.9f, 0.7f}, // warm λάμπα
            {1.0f, 1.0f, 1.0f},
            0.22f, // aggressive linear falloff
            0.20f // aggressive quadratic
        };

        // our_dir_light={
        // {1.0f, 0.0f, -1.0f},
        // {0.1f, 0.1f, 0.1f},  // ambient - carries most of the visibility
        // {0.025f, 0.025f, 0.025f},  // diffuse - barely contributes
        // {0.0f, 0.0f, 0.0f}   // specular - off
        // };
        //
        // our_dir_light = {
        //     {1.0f, 0.0f, -1.0f},
        //     {0.01f, 0.01f, 0.01f},  // ambient - barely exists
        //     {0.15f, 0.14f, 0.13f},  // diffuse - reveals shape, slightly warm
        //     {0.0f, 0.0f, 0.0f}      // specular - off
        // };

        //Renderer_Register_Point_Light(our_light);


        our_dir_light = {
            {1.0f, -1.0f, -1.0f}, // direction - angled down and forward, easier to see volume

            {0.2f, 0.2f, 0.2f}, // ambient - lifted so shadow areas aren't black
            {1.0f, 0.95f, 0.9f}, // diffuse - close to white, strong
            {0.8f, 0.8f, 0.8f} // specular - strong highlights
        };

        Renderer_Register_Spot_Light(our_spot_light);
        Renderer_Register_Directional_Light(our_dir_light);

        Transform t{};
        t.Position = our_light.position;
        mesh_instance lights[]{
            light_source, t
        };

        Renderer_FinalizeMeshLoading();

        bool keepRunning = true;

        Material our_material{
            2
        };


        Uint64 freq = SDL_GetPerformanceFrequency(); // do this once, outside the loop
        Uint64 lastTime = SDL_GetPerformanceCounter(); // do this once, outside the loop
        double ms_per_frame = 0.0;
        int objects = 0;
        static char debug_text[1024] = {};


        Transform trans = {
            {0, 1, -10},
            {DegreeToRadians(0), 0, 0},
            {0.25, 0.25, 0.25}

        };

        const double max_delta = 1000.0 / 30.0;


        // TODO we can think later how to pick proper planes and what not
        // we can always fix this later
        // for now const will do
        const Vector3D plane_normal = {0, 1, 0};
        const Vector3D plane_point = grid_origin_2D;

        while (keepRunning) {
            Uint64 now = SDL_GetPerformanceCounter();
            double ms = (now - lastTime) * 1000.0 / freq;
            const double delta = (ms > max_delta ? max_delta : ms);
            lastTime = now;
            ms_per_frame = 0.1 * ms + 0.9 * ms_per_frame;
            objects = Vector_mesh_instance_Length(opaque_meshes);


            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                switch (event.type) {
                    case SDL_EVENT_WINDOW_RESIZED: {
                        Renderer_ResolutionChanged(event.window.data1, event.window.data2);
                        width = event.window.data1;
                        height = event.window.data2;
                        break;
                    }
                    case SDL_EVENT_WINDOW_CLOSE_REQUESTED: {
                        keepRunning = false;
                        break;
                    }
                    case SDL_EVENT_KEY_DOWN: {
                        KeyDown(event.key.scancode, *SceneCamera);
                        break;
                    }
                    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
                        bool left_pressed = event.button.button == SDL_BUTTON_LEFT;
                        bool right_pressed = event.button.button == SDL_BUTTON_RIGHT;

                        if (!left_pressed && !right_pressed) {
                            break;
                        }

                        sdf_operation_smooth op = left_pressed ? union_sdf_smooth : subtraction_sdf_smooth;

                        float x_pixel = event.button.x;
                        float y_pixel = event.button.y;


                        const Ray3D mouse_ray = pixel_to_ray_in_world_space(
                            SceneCamera,
                            width,
                            height,
                            x_pixel,
                            y_pixel, 45
                        );


                        const Vector3D intersect_at = find_ray_and_plane_intersection(
                            mouse_ray,
                            plane_normal,
                            plane_point
                        );

                        const float brush_radius = 1.0;
                        const float k_factor=step_2D;
                        const float edge_offset=brush_radius+k_factor+step_2D;
                        const float cx_max = intersect_at.x + edge_offset;
                        const float cx_min = intersect_at.x - edge_offset;

                        // world_x= grid_origin_x + i * step;
                        // therefore i= (world_x-grid_origin_x)/step;

                        // handle undershooting with max
                        const int index_x_min = math_ops::max(
                            math_ops::ceil_to_int((cx_min - grid_origin_2D.x) / step_2D),
                            0
                        );

                        // handle overshooting with min
                        const int index_x_max = math_ops::min(
                            math_ops::floor_to_int((cx_max - grid_origin_2D.x) / step_2D),
                            samples_per_axis_2D - 1
                        );

                        const float cz_max = intersect_at.z + edge_offset;
                        const float cz_min = intersect_at.z - edge_offset;

                        const int index_z_min = math_ops::max(
                            math_ops::ceil_to_int((cz_min - grid_origin_2D.z) / step_2D),
                            0
                        );

                        // handle overshooting with min
                        const int index_z_max = math_ops::min(
                            math_ops::floor_to_int((cz_max - grid_origin_2D.z) / step_2D),
                            samples_per_axis_2D - 1
                        );

                        Vector2D circle_center{intersect_at.x, intersect_at.z};

                        for (int z = index_z_min; z <= index_z_max; ++z) {
                            for (int x = index_x_min; x <= index_x_max; ++x) {
                                size_t index = z * samples_per_axis_2D + x;
                                //
                                Vector3D point_in_world_space = grid_origin_2D;
                                point_in_world_space.x += x * step_2D;
                                point_in_world_space.z += z * step_2D;
                                const Vector2D v{point_in_world_space.x, point_in_world_space.z};
                                float brush_sdf = sample_cirlce_density(v,circle_center, brush_radius);

                                samples[index] = op(samples[index], brush_sdf,k_factor);
                            }
                        }

                        contour_line_count = 0;
                        sample_contours(samples, contour_lines, contour_line_count);
                        DEBUG_SendContourDataToTheGPU(contour_lines, contour_line_count);
                        DEBUG_SendPointDataToTheGPU(points, samples, total_sampling_points_2D);


                        break;
                    }

                    default: {
                        break;
                    }
                }

                ImGui_ImplSDL3_ProcessEvent(&event); // Forward your event to backend
            }

            float x_pixel;
            float y_pixel;
            SDL_GetMouseState(&x_pixel, &y_pixel);

            // we get the end of a ra
            Ray3D mouse_ray =
                    pixel_to_ray_in_world_space(SceneCamera, width, height, x_pixel, y_pixel, 45);


            const Vector3D intersect_at = find_ray_and_plane_intersection(
                mouse_ray,
                plane_normal,
                plane_point
            );


            DEBUG_Update_Brush_Trans(intersect_at);


            Renderer_FrameStart();


            // for (size_t i = 0; i < Vector_mesh_instance_Length(opaque_meshes); ++i) {
            //     const mesh_instance m = opaque_meshes->data[i];
            //     Renderer_Draw_Mesh(m.mesh_id, m.transform, {1, 1, 1}, our_material);
            // }

            //Renderer_Draw_Skeletal_Model(bob, trans, {}, {});

            Renderer_ResolveDrawCalls(delta);

            // // Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplSDL3_NewFrame();
            ImGui::NewFrame();
            // //
            IM_ASSERT(ImGui::GetCurrentContext() != nullptr && "Missing Dear ImGui context. Refer to examples app!");
            // //
            // // // Verify ABI compatibility between caller code and compiled version of Dear ImGui. This helps detects some build issues.
            IMGUI_CHECKVERSION();

            snprintf(debug_text, sizeof(debug_text), "ms: %f\n objects:%d\n", ms_per_frame, objects);

            bool up = ImGui::ArrowButton("Increase z", ImGuiDir_Up);
            ImGui::SameLine();
            bool down = ImGui::ArrowButton("Decrease z", ImGuiDir_Up);
            if (up) {
                DEBUG_Modify_Slice_Layer(1);
            } else if (down) {
                DEBUG_Modify_Slice_Layer(-1);
            }


            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            // // Update and Render additional Platform Windows
            // // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
            // //  For this specific demo app we could also call SDL_GL_MakeCurrent(window, gl_context) directly)
            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
                SDL_Window *backup_current_window = SDL_GL_GetCurrentWindow();
                const SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext(); // NOLINT(*-misplaced-const) , we want this as is
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
                SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
            }

            Renderer_FrameEnd();
        }


        Renderer_Destroy();

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
    } catch (const std::exception &e) {
        fprintf(stderr, "Fatal error: %s", e.what());
        return EXIT_FAILURE;
    }

    return 0;
}
