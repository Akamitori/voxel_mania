#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 mat_row_0;
layout (location = 2) in vec4 mat_row_1;
layout (location = 3) in vec4 mat_row_2;
layout (location = 4) in vec4 mat_row_3;
layout (location = 5) in vec3 voxel_color;



smooth out vec4 theColor;
uniform mat4 perspectiveMatrix;


void main() {

    mat4 model_space_transforms = mat4(mat_row_0, mat_row_1, mat_row_2, mat_row_3);
    
    mat4 final_perspective_matrix = perspectiveMatrix * model_space_transforms;

    vec4 final_pos = final_perspective_matrix * vec4(aPos, 1.0);

    gl_Position = final_pos;
    
    theColor = vec4(voxel_color, 1.0);
}