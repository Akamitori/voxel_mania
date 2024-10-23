#version 330 core

layout (location = 0) in vec3 aPos;


smooth out vec4 theColor;

uniform vec4 voxel_color;
uniform mat4 perspectiveMatrix;


void main() {

    vec4 final_pos=perspectiveMatrix* vec4(aPos, 1.0);

    gl_Position = final_pos;
    
    vec3 color_pos=final_pos.xyz;
    
    vec3 colorX1 = vec3(voxel_color.r, 0, 0);
    vec3 colorX2 = vec3(0, voxel_color.g, 0);

    vec3 colorY1 = vec3(0, voxel_color.g, 0);
    vec3 colorY2 = vec3(0, 0, voxel_color.b);

    vec3 colorZ1 = vec3(voxel_color.r, voxel_color.b, 0);
    vec3 colorZ2 = vec3(0, voxel_color.g, voxel_color.b);

    float interpX = (aPos.x + 1.0) * 0.5;
    float interpY = (aPos.y + 1.0) * 0.5;
    float interpZ = (aPos.z + 1.0) * 0.5;

    // Interpolate colors for each axis
    vec3 colorX = mix(colorX1, colorX2, interpX);
    vec3 colorY = mix(colorY1, colorY2, interpY);
    vec3 colorZ = mix(colorZ1, colorZ2, interpZ);

    vec3 finalColor = (colorX + colorY + colorZ) / 3.0;


    theColor = vec4(finalColor,1.0);
}