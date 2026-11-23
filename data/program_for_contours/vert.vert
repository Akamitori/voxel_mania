#version 330 core

layout (location = 0) in vec3 aPos;


layout (std140) uniform ViewMatrices{
    mat4 perspective_projection_matrix;// 16*4 bytes
    mat4 look_at_matrix;// 16*4 bytes
};



out vec3 PointColor;
uniform vec3 color;

void main() {

//    if (z_layer!=0 && abs(aPos.z - z_layer) > 0.5 * 0.5){
//        gl_Position=vec4(2.0,2.0,2.0,1.0);
//        return;
//    }

    gl_Position = perspective_projection_matrix*look_at_matrix * vec4(aPos, 1.0);
    PointColor = color;
}