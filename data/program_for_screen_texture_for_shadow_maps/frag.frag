#version 330 core

out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2DArray depthMap;
uniform int layer;

void main()
{
    vec3 sample_coords=vec3(TexCoord.xy,layer);
    float depthValue = texture(depthMap, sample_coords).r;

    float near = 0.1;
    float far = 100.0;
    float linearDepth = (2.0 * near) / (far + near - depthValue * (far - near));
    FragColor = vec4(vec3(linearDepth), 1.0);
}
