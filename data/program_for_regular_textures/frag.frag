#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform vec4 voxel_color;
uniform vec3 view_position;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    sampler2D emission;
    float shininess;
};

uniform Material material;

struct PointLight{
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float linear_factor;
    float quadric_factor;
};

struct DirectionalLight{
    vec3 direction;// 16
    vec3 ambient;// 16
    vec3 diffuse;// 16
    vec3 specular;// 16
// no padding
};

struct SpotLight{
    vec3 position;//16
    vec3 direction;// 16
    vec3 ambient;// 16
    vec3 diffuse;// 16
    vec3 specular;// 16
    float linear_factor;//4
    float quadric_factor;//4
    float inner_cutoff;//4
    float outer_cutoff;//4
// 4 padding
};

const int MAX_DIRECTIONAL_LIGHTS=4;

layout (std140) uniform Directional_Lights{
    int numDirectionalLights;// 16 
    DirectionalLight directional_lights[MAX_DIRECTIONAL_LIGHTS];// 64*max lights
};

const int MAX_POINT_LIGHTS=4;

layout (std140) uniform Point_Lights{
    int numPointLights;// 16 
    PointLight point_lights[MAX_POINT_LIGHTS];// 64*max lights
};

const int MAX_SPOT_LIGHTS=4;

layout (std140) uniform Spot_Lights{
    int numSpotLights;// 16 
    SpotLight spot_lights[MAX_SPOT_LIGHTS];// 64*max lights
};

vec3 CalculateDirectionalLights(vec3 diffuseTexMap, vec3 specularTexMap, vec3 norm);
vec3 CalculatePointLights(vec3 diffuseTexMap, vec3 specularTexMap, vec3 norm);
vec3 CalculateSpotLights(vec3 diffuseTexMap, vec3 specularTexMap, vec3 norm);

void main() {
    vec3 diffuseTexMap=  vec3(texture(material.diffuse, TexCoord));
    vec3 specularTexMap = vec3(texture(material.specular, TexCoord));
    vec3 emisionTexMap =   vec3(texture(material.emission, TexCoord));
    vec3 norm=normalize(Normal);

    vec3 output_color=vec3(0.0);

    output_color+=CalculateDirectionalLights(diffuseTexMap, specularTexMap, norm);
    output_color+=CalculatePointLights(diffuseTexMap, specularTexMap, norm);
    output_color+=CalculateSpotLights(diffuseTexMap, specularTexMap, norm);
    output_color+=emisionTexMap;

    FragColor= vec4(output_color, 1.0);
}

vec3 CalculatePointLights(vec3 diffuseTexMap, vec3 specularTexMap, vec3 norm){
    vec3 output_color=vec3(0.0);
    vec3 viewDir=normalize(view_position-FragPos);
    for (int i=0;i< numPointLights;++i){
        PointLight light = point_lights[i];
        vec3 ambient = light.ambient * diffuseTexMap;

        vec3 light_direction_vector=light.position-FragPos;

        vec3 light_direction=normalize(light_direction_vector);

        float diff= max(dot(norm, light_direction), 0.0);
        vec3 diffuse = light.diffuse * diff * diffuseTexMap;

        // specular
        vec3 reflectionDirection=reflect(-light_direction, norm);
        float spec=pow(max(dot(viewDir, reflectionDirection), 0.0), material.shininess);
        vec3 specular= light.specular * spec* specularTexMap;

        float distance= length(light_direction_vector);

        float attenuation = 1.0 / (1.0 + light.linear_factor * distance +
        light.quadric_factor * (distance * distance));

        output_color+= (ambient+diffuse + specular)*attenuation;
    }
    return output_color;
}

vec3 CalculateDirectionalLights(vec3 diffuseTexMap, vec3 specularTexMap, vec3 norm){
    vec3 output_color=vec3(0.0);
    vec3 viewDir=normalize(view_position-FragPos);
    for (int i=0;i< numDirectionalLights;++i){
        DirectionalLight light = directional_lights[i];
        vec3 ambient = light.ambient * diffuseTexMap;

        vec3 light_direction=normalize(-light.direction);

        float diff= max(dot(norm, light_direction), 0.0);
        vec3 diffuse = light.diffuse * diff * diffuseTexMap;

        // specular
        vec3 reflectionDirection=reflect(-light_direction, norm);
        float spec=pow(max(dot(viewDir, reflectionDirection), 0.0), material.shininess);
        vec3 specular= light.specular * spec* specularTexMap;

        output_color+= ambient+ diffuse + specular;
    }
    return output_color;
}

vec3 CalculateSpotLights(vec3 diffuseTexMap, vec3 specularTexMap, vec3 norm){
    vec3 output_color=vec3(0.0);
    vec3 viewDir=normalize(view_position-FragPos);
    for (int i=0;i< numSpotLights;++i){
        SpotLight light = spot_lights[i];

        vec3 light_direction_vector=light.position-FragPos;
        vec3 light_direction=normalize(light_direction_vector);
        float theta = dot(light_direction, normalize(-light.direction));

        float epsilon   = light.inner_cutoff - light.outer_cutoff;
        float intensity = clamp((theta - light.outer_cutoff) / epsilon, 0.0, 1.0);


        vec3 ambient = light.ambient * diffuseTexMap;

        float diff= max(dot(norm, light_direction), 0.0);
        vec3 diffuse = light.diffuse * diff * diffuseTexMap;

        // specular
        vec3 reflectionDirection=reflect(-light_direction, norm);
        float spec=pow(max(dot(viewDir, reflectionDirection), 0.0), material.shininess);
        vec3 specular= light.specular * spec* specularTexMap;

        float distance= length(light_direction_vector);

        float attenuation = 1.0 / (1.0 + light.linear_factor * distance +light.quadric_factor * (distance * distance));

        // remove attenuation from ambient, as otherwise at large distances the light would be darker inside than outside the spotlight due the ambient term in the else branch
        output_color+= (ambient+diffuse + specular)*attenuation*intensity;
    }
    return output_color;
}





