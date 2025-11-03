#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D ourTexture;
uniform vec4 voxel_color;
uniform vec3 view_position;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

uniform Material material;

struct Light{
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Light light;

void main() {
    // ambient
    vec3 ambient=light.ambient* material.ambient;

    // diffuse
    vec3 norm=normalize(Normal);
    vec3 light_direction=normalize(light.position-FragPos);
    float diff= max(dot(norm, light_direction), 0.0);
    vec3 diffuse=light.diffuse * diff * material.diffuse;

    // specular
    vec3 viewDir=normalize(view_position-FragPos);
    vec3 reflectionDirection=reflect(-light_direction, norm);
    float spec=pow(max(dot(viewDir, reflectionDirection), 0.0), material.shininess);
    vec3 specular= light.specular * spec* material.specular;

    // combined light
    vec4 combined_light_color= vec4(ambient+ diffuse + specular, 1.0);

    vec4 texColor=texture(ourTexture, TexCoord);
    vec4 object_color=texColor * voxel_color;

    FragColor= combined_light_color * object_color;
}