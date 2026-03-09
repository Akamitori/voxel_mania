#version 330 core

out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2DArray depthMap;
uniform int layer;

void main()
{
    vec3 sample_coords=vec3(TexCoord.xy,layer);
    float depthValue = texture(depthMap, sample_coords).r;
    FragColor = vec4(vec3(depthValue), 1.0);
}
