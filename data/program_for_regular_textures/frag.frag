#version 330 core
out vec4 FragColor;

in VS_OUT{
    vec2 TexCoord;
    vec3 Normal;
    vec3 FragPosWorldSpace;
    float[3] u_products_for_interpolation;
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
    mat4 mvp_matrix_for_cascade[MAX_DIRECTIONAL_LIGHTS*MAX_SHADOW_CASCADES];   // 64*max lights
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


uniform sampler2DArray shadowMap;
uniform bool usePCF;

vec3 CalculateDirectionalLights(vec3 diffuseTexMap, vec3 specularTexMap, vec3 normal, vec3 fragPos);
vec3 CalculatePointLights(vec3 diffuseTexMap, vec3 specularTexMap, vec3 normal, vec3 fragPos);
vec3 CalculateSpotLights(vec3 diffuseTexMap, vec3 specularTexMap, vec3 normal, vec3 fragPos);
float CalculateShadowFactor(vec3 frag_pos_world_space, vec3 normal, vec3 light_dir);

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


float CalculateShadowFactor(vec3 frag_pos_world_space, vec3 normal, vec3 light_dir){
    // perspective divide for the frag pos
    
    // convert the frag pos to light space here!
    // then find out which cascade to use
    // then wow we are done :O
    float depth = texture(shadowMap, vec3(0, 0, float(0))).r;
    return 0;
//    
//    vec4 frag_pos_light_space=vec4(frag_pos_world_space,1);
//    
//    vec3 projCoords=frag_pos_light_space.xyz/frag_pos_light_space.w;
//
//    // outside the far plane. consider this without a shadow
//    if (projCoords.z > 1.0){
//        return 0.0;
//    }
//
//    // map the x and y cords from [-1,1] to [0,1] for texture sampling
//    // we don't need to map z because it is is already mapped due to how 
//    // our matrix is setup on the cpu side
//    vec2 shadowUV= projCoords.xy*0.5+0.5;
//
//    // our current depth based on calcs
//    float currentDepth = projCoords.z;
//    
//    // calculate a bias based on the light direction  
//    // so we can deal with shadow acne
//    float bias = max(0.05 * (1.0 - dot(normal, light_dir)), 0.005);
//    
//    // the value we will compare against to see if the fragment is in shadow or not
//    float depth_compare_value=currentDepth - bias;
//    if (!usePCF){
//        float closestDepth = texture(shadowMap, shadowUV).r;
//        float shadow = depth_compare_value > closestDepth  ? 1.0 : 0.0;
//        return shadow;
//    }
//
//    float shadow = 0;
//
//    // calculate texture element size
//    vec2 texel_size=1.0/textureSize(shadowMap, 0);
//    
//    // assume a grid_size x grid_sizer kernel for pcf
//    // in the future we can try other ways to sample
//    const int grid_size=3;
//    
//    const int x_start=-grid_size;
//    const int x_end=grid_size;
//
//    const int y_start=-grid_size;
//    const int y_end=grid_size;
//
//    const int sample_count= (x_end-x_start+1) * (y_end-y_start+1);
//
//    for (int x=x_start;x<=x_end;++x){
//        for (int y=y_start;y<=y_end;++y){
//            vec2 offset_texture_coords=shadowUV+vec2(x, y)*texel_size;
//            float pcfDepth= texture(shadowMap, offset_texture_coords).r;
//            float pcfShadow=depth_compare_value > pcfDepth ? 1.0: 0.0;
//            shadow+=pcfShadow;
//        }
//    }
//    shadow/=sample_count;
//
//    return shadow;
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

        // for each light we need to get the actual texture
        // for now it's hardcoded but let's not forget that!
        float shadow_factor=CalculateShadowFactor(fs_in.FragPosWorldSpace, normal, light_direction);

        shadow_factor=0;
        float diff= max(dot(normal, light_direction), 0.0);
        vec3 diffuse = light.diffuse * diff * diffuseTexMap;

        // specular
        vec3 halfwayDir = normalize(light_direction + viewDir);
        float spec=pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
        vec3 specular= light.specular * spec* specularTexMap;

        output_color+= ambient+ (1.0-shadow_factor)*(diffuse + specular);
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




