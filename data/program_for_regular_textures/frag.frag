#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D ourTexture;
uniform vec4 voxel_color;
uniform vec3 light_color;
uniform vec3 light_position;
uniform vec3 view_position;

void main() {
    vec3 norm=normalize(Normal);
    vec3 light_direction=normalize(light_position-FragPos);

    // diffuse impact
    float diff= max(dot(norm, light_direction), 0.0);
    vec3 diffuse_color=diff*light_color;


    vec4 texColor=texture(ourTexture, TexCoord);

    float ambientStrength = 0.1;
    vec3 ambient_light_color=vec3(light_color * ambientStrength);

    float specularStrength=1;

    vec3 viewDir=normalize(view_position- FragPos);
    vec3 reflectDir= reflect(-light_direction, norm);

    float spec=pow(max(dot(viewDir, reflectDir), 0.0), 64);
    vec3 specular= specularStrength *spec * light_color;
    
    vec4 combined_light_color= vec4(ambient_light_color+ diffuse_color + specular, 1.0);

    vec4 object_color=texColor * voxel_color;

    FragColor= combined_light_color * object_color;
}