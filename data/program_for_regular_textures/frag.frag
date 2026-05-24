#version 330 core
out vec4 FragColor;

in VS_OUT{
    vec2 TexCoord;
    vec3 Normal;
    vec3 FragPosWorldSpace;
    vec3 u_products_for_interpolation;
    vec3 cascade_coord_0;
} fs_in;

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
};

const int MAX_DIRECTIONAL_LIGHTS=1;
const int MAX_SHADOW_CASCADES=4;

layout (std140) uniform Directional_Lights{
    int numDirectionalLights;// 16 
    DirectionalLight directional_lights[MAX_DIRECTIONAL_LIGHTS];// 64*max lights
    mat4 mvp_matrix_for_cascade[MAX_DIRECTIONAL_LIGHTS*MAX_SHADOW_CASCADES];// 64*max lights
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

uniform sampler2DArrayShadow shadowMap;
uniform vec3 cascade_scale[3];
uniform vec3 cascade_offset[3];
uniform vec4 shadow_offset[2];

vec3 CalculateDirectionalLights(vec3 diffuseTexMap, vec3 specularTexMap, vec3 normal, vec3 fragPos);
vec3 CalculatePointLights(vec3 diffuseTexMap, vec3 specularTexMap, vec3 normal, vec3 fragPos);
vec3 CalculateSpotLights(vec3 diffuseTexMap, vec3 specularTexMap, vec3 normal, vec3 fragPos);
float CalculateShadowFactor(vec3 frag_pos_world_space, vec3 normal, vec3 light_dir);
float saturate(float value);

void main() {
    vec4 texture_color=texture(material.diffuse, fs_in.TexCoord);

    vec3 diffuseTexMap=  vec3(texture_color);
    vec3 specularTexMap = vec3(texture(material.specular, fs_in.TexCoord));
    vec3 emisionTexMap =   vec3(texture(material.emission, fs_in.TexCoord));
    vec3 normal=normalize(fs_in.Normal);

    vec3 output_color=vec3(0.0);
    output_color+=CalculateDirectionalLights(diffuseTexMap, specularTexMap, normal, fs_in.FragPosWorldSpace);
    output_color+=CalculatePointLights(diffuseTexMap, specularTexMap, normal, fs_in.FragPosWorldSpace);
    output_color+=CalculateSpotLights(diffuseTexMap, specularTexMap, normal, fs_in.FragPosWorldSpace);
    output_color+=emisionTexMap;

    FragColor= vec4(output_color, texture_color.a);
}

float saturate(float value){
    return clamp(value, 0, 1);
}

vec3 saturate(vec3 value){
    return vec3(saturate(value.x), saturate(value.y), saturate(value.z));
}

float CalculateInfiniteShadow(vec3 cascadeCoord0, vec3 cascadeBlend, vec3 normal, vec3 light_dir){
    vec3 p1;
    vec3 p2;

    // apply scales and offsets to get text coords for all four cascades
    vec3 cascadeCoord1=cascadeCoord0* cascade_scale[0] + cascade_offset[0];
    vec3 cascadeCoord2=cascadeCoord0* cascade_scale[1] + cascade_offset[1];
    vec3 cascadeCoord3=cascadeCoord0* cascade_scale[2] + cascade_offset[2];
    
    // calculate layer indices i and j
    bool beyond_cascade_2= cascadeBlend.y>=0;
    bool beyond_cascade_3= cascadeBlend.z>=0;
    p1.z=  float(beyond_cascade_2)*2.0;
    p2.z=  float(beyond_cascade_3)*2.0+1.0;

    vec2 shadow_coord_1=(beyond_cascade_2) ? cascadeCoord2.xy: cascadeCoord0.xy;
    vec2 shadow_coord_2=(beyond_cascade_3) ? cascadeCoord3.xy: cascadeCoord1.xy;
    float depth1= (beyond_cascade_2) ? cascadeCoord2.z : cascadeCoord0.z;
    float depth2= (beyond_cascade_3) ? saturate(cascadeCoord3.z) : cascadeCoord1.z;

    vec3 blend=saturate(cascadeBlend);
    float weight= (beyond_cascade_2) ? blend.y- blend.z : 1.0 - blend.x;
    
    // fetch four samples from the first cascade
    p1.xy= shadow_coord_1+shadow_offset[0].xy;
    float light1= texture(shadowMap, vec4(p1, depth1));
    p1.xy= shadow_coord_1 + shadow_offset[0].zw;
    light1+= texture(shadowMap, vec4(p1, depth1));
    p1.xy= shadow_coord_1+shadow_offset[1].xy;
    light1+= texture(shadowMap, vec4(p1, depth1));
    p1.xy= shadow_coord_1 + shadow_offset[1].zw;
    light1+= texture(shadowMap, vec4(p1, depth1));

    // fetch four samples from the second cascade
    p2.xy= shadow_coord_2+shadow_offset[0].xy;
    float light2= texture(shadowMap, vec4(p2, depth2));
    p2.xy= shadow_coord_2 + shadow_offset[0].zw;
    light2+= texture(shadowMap, vec4(p2, depth2));
    p2.xy= shadow_coord_2+shadow_offset[1].xy;
    light2+= texture(shadowMap, vec4(p2, depth2));
    p2.xy= shadow_coord_2 + shadow_offset[1].zw;
    light2+= texture(shadowMap, vec4(p2, depth2));

    float blended_value=mix(light2, light1, weight);
    float dividant=0.25;

    return blended_value* dividant;
}

vec3 CalculatePointLights(vec3 diffuseTexMap, vec3 specularTexMap, vec3 normal, vec3 fragPos){
    vec3 output_color=vec3(0.0);
    vec3 viewDir=normalize(view_position-fragPos);
    for (int i=0;i< numPointLights;++i){
        PointLight light = point_lights[i];
        vec3 ambient = light.ambient * diffuseTexMap;

        vec3 light_direction_vector=light.position-fragPos;

        vec3 light_direction=normalize(light_direction_vector);

        float diff= max(dot(normal, light_direction), 0.0);
        vec3 diffuse = light.diffuse * diff * diffuseTexMap;

        // specular
        vec3 halfwayDir = normalize(light_direction + viewDir);
        float spec=pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
        vec3 specular= light.specular * spec* specularTexMap;

        float distance= length(light_direction_vector);

        float attenuation = 1.0 / (1.0 + light.linear_factor * distance +
        light.quadric_factor * (distance * distance));

        output_color+= (ambient+diffuse + specular)*attenuation;
    }
    return output_color;
}

vec3 CalculateDirectionalLights(vec3 diffuseTexMap, vec3 specularTexMap, vec3 normal, vec3 fragPos){
    vec3 output_color=vec3(0.0);
    vec3 viewDir=normalize(view_position-fragPos);



    for (int i=0;i< numDirectionalLights;++i){
        DirectionalLight light = directional_lights[i];
        vec3 ambient = light.ambient * diffuseTexMap;

        vec3 light_direction=normalize(-light.direction);

        float shadow_factor=CalculateInfiniteShadow(fs_in.cascade_coord_0, fs_in.u_products_for_interpolation, normal, light_direction);

        float diff= max(dot(normal, light_direction), 0.0);
        vec3 diffuse = light.diffuse * diff * diffuseTexMap;

        // specular
        vec3 halfwayDir = normalize(light_direction + viewDir);
        float spec=pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
        vec3 specular= light.specular * spec* specularTexMap;

        output_color+= ambient+ (shadow_factor)*(diffuse + specular);
    }
    return output_color;
}

vec3 CalculateSpotLights(vec3 diffuseTexMap, vec3 specularTexMap, vec3 normal, vec3 fragPos){
    vec3 output_color=vec3(0.0);
    vec3 viewDir=normalize(view_position-fragPos);
    for (int i=0;i< numSpotLights;++i){
        SpotLight light = spot_lights[i];

        vec3 light_direction_vector=light.position-fragPos;
        vec3 light_direction=normalize(light_direction_vector);
        float theta = dot(light_direction, normalize(-light.direction));

        float epsilon   = light.inner_cutoff - light.outer_cutoff;
        float intensity = clamp((theta - light.outer_cutoff) / epsilon, 0.0, 1.0);


        vec3 ambient = light.ambient * diffuseTexMap;

        float diff= max(dot(normal, light_direction), 0.0);
        vec3 diffuse = light.diffuse * diff * diffuseTexMap;

        // specular
        vec3 halfwayDir = normalize(light_direction + viewDir);
        float spec=pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
        vec3 specular= light.specular * spec* specularTexMap;

        float distance= length(light_direction_vector);

        float attenuation = 1.0 / (1.0 + light.linear_factor * distance +light.quadric_factor * (distance * distance));

        // remove attenuation from ambient, as otherwise at large distances the light would be darker inside than outside the spotlight due the ambient term in the else branch
        output_color+= (ambient+diffuse + specular)*attenuation*intensity;
    }
    return output_color;
}




