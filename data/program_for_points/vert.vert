#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in float aDensity;


layout (std140) uniform ViewMatrices{
    mat4 perspective_projection_matrix;// 16*4 bytes
    mat4 look_at_matrix;// 16*4 bytes
};



out vec3 PointColor;
uniform float z_layer;

void main() {

    if (abs(aPos.z - z_layer) > 0.5 * 0.5){
        gl_Position=vec4(2.0,2.0,2.0,1.0);
        return;
    }

    gl_Position = perspective_projection_matrix*look_at_matrix * vec4(aPos, 1.0);



    int g=int(aDensity<=0);
    int r=int(aDensity>0);



    float max_distance=15;



    float distance_factor=1.0-clamp(abs(aDensity)/max_distance,0.0,1.0);




    PointColor = vec3(r,g,0) * distance_factor;
}