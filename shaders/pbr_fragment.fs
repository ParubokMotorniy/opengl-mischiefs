#version 460 core

out vec4 FragColor;

// material parameters
uniform sampler2D albedoMap;
uniform sampler2D roughnessMap;
uniform sampler2D metallicMap;
uniform sampler2D normalMap;
uniform sampler2D aoMap;

uniform vec3 viewPos;

struct DirectionalLight
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    vec3 direction;
    vec3 position;

    mat4 viewMatrix;
    mat4 projectionMatrix;

    //TODO: these ideally shouldn't be here
    uvec2 shadowMapIdentifier;
    uvec2 frameBufferId;
};

struct PointLight
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float attenuationConstantTerm;
    float attenuationLinearTerm;
    float attenuationQuadraticTerm;

    vec3 position;
};

struct SpotLight
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float innerCutOff;
    float outerCutOff;

    float attenuationConstantTerm;
    float attenuationLinearTerm;
    float attenuationQuadraticTerm;

    vec3 position;
    vec3 direction;
};

struct TexturedSpotLight
{
    vec3 ambient;
    
    vec3 specular;
    float innerCutOff;
    
    float outerCutOff;
    float attenuationConstantTerm;
    float attenuationLinearTerm;
    float attenuationQuadraticTerm;
    
    vec3 position;
    vec3 direction;

    mat4 lightView;
    mat4 lightProj;

    uvec2 textureIdx;
};

#define NUM_DIRECTIONAL 8
layout(binding = 1, std140) uniform DirectionalLights
{
    DirectionalLight dirLights[NUM_DIRECTIONAL];
};
uniform int numDirectionalLightsBound;

#define NUM_POINT 8
layout(binding = 2, std140) uniform PointLights { PointLight pointLights[NUM_POINT]; };
uniform int numPointLightsBound;

#define NUM_SPOT 8
layout(binding = 3, std140) uniform SpotLights { SpotLight spotLights[NUM_SPOT]; };
uniform int numSpotLightsBound;

#define NUM_TEXTURED_SPOT 4
layout(binding = 4, std140) uniform TexturedSpotLights { TexturedSpotLight texturedSpotLights[NUM_TEXTURED_SPOT]; };
uniform int numTexturedLightsBound;

uniform sampler2DShadow directionalShadowMaps[NUM_DIRECTIONAL];
// uniform sampler2D directionalShadowMaps[NUM_DIRECTIONAL];
uniform float directionalShadowBias;

const float PI = 3.14159265359;

in VertexParamPack
{
    vec2 texCoord;
    vec3 vPos;
    mat4 tbnMatrix;
} fs_in;
  
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}
float GeometrySmith(vec3 N, vec3 viewDir, vec3 L, float roughness)
{
    float NdotV = max(dot(N, viewDir), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
} 

float fragmentInDirectionalShadow(DirectionalLight light, int lightIdx, vec3 fragWorldPos, vec3 norm)
{
    vec3 displacedFragment = fragWorldPos.xyz + norm * directionalShadowBias; 
    vec4 ndcPos = light.projectionMatrix * light.viewMatrix * vec4(displacedFragment, 1.0);
    ndcPos /= ndcPos.w;

    ndcPos = ndcPos * 0.5 + 0.5;
    float fragmentDepth = ndcPos.z;

    if(ndcPos.z > 1.0)
        return 0.0;

    //hardware pcf 
    // float shadowDepth = texture(directionalShadowMaps[lightIdx], ndcPos.xyz).r;
    // return 1.0 - shadowDepth;

    //software pcf
    // vec2 texelSize = 1.0 / textureSize(directionalShadowMaps[lightIdx], 0);
    // float shadow = 0.0;
    // for(int x = -1; x <= 1; ++x)
    // {
    //     for(int y = -1; y <= 1; ++y)
    //     {
    //         float pcfDepth = texture(directionalShadowMaps[lightIdx], ndcPos.xy + vec2(x, y) * texelSize).r; 
    //         shadow += fragmentDepth > pcfDepth ? 1.0 : 0.0;        
    //     }    
    // }
    // shadow /= 9.0;
    // return shadow;

    //hardware + software pcf
    vec2 texelSize = 1.0 / textureSize(directionalShadowMaps[lightIdx], 0);
    float shadow = 0.0;
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float shadowDepth = texture(directionalShadowMaps[lightIdx], vec3(ndcPos.xy + vec2(x, y) * texelSize, ndcPos.z)).r;
            shadow += (1.0 - shadowDepth);      
        }    
    }
    shadow /= 9.0;
    return shadow;

    //no pcf
    // float shadowDepth = texture(directionalShadowMaps[lightIdx], ndcPos.xy).r;
    // float shadow = fragmentDepth > shadowDepth ? 1.0 : 0.0;
    // return shadow;
}

vec3 CalculateDirectionalLightRadianceContribution(DirectionalLight light, int lightIdx, vec3 normal, vec3 viewDir, vec3 albedo, vec3 F0, float metallic, float roughness, vec3 fPos)
{
    // calculate per-light radiance
    vec3 L = normalize(-light.direction);
    vec3 H = normalize(viewDir + L);
    // float shadowEffect = 1.0 - fragmentInDirectionalShadow(light, lightIdx, fPos, normal);
    vec3 radiance     = light.diffuse; //*shadowEffect        
    
    // cook-torrance brdf
    float NDF = DistributionGGX(normal, H, roughness);        
    float G   = GeometrySmith(normal, viewDir, L, roughness);      
    vec3 F    = fresnelSchlick(max(dot(H, viewDir), 0.0), F0);       
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;	  
    
    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, L), 0.0) + 0.0001;
    vec3 specular     = numerator / denominator;  
        
    float NdotL = max(dot(normal, L), 0.0);                
    return (kD * albedo / PI + specular) * radiance * NdotL; 
}

vec3 CalculatePointLightRadianceContribution(PointLight light, vec3 normal, vec3 viewDir, vec3 albedo, vec3 F0, float metallic, float roughness, vec3 fPos)
{
    vec3 L = normalize(light.position - fPos);
    vec3 H = normalize(viewDir + L);
    float distance = length(light.position - fPos);
    float attenuation = clamp(1.0
                                  / (light.attenuationConstantTerm
                                     + light.attenuationLinearTerm * distance
                                     + light.attenuationQuadraticTerm * (distance * distance)),
                              0.0, 1.0);
    vec3 radiance     = light.diffuse * attenuation;      
    
    // cook-torrance
    float NDF = DistributionGGX(normal, H, roughness);        
    float G   = GeometrySmith(normal, viewDir, L, roughness);      
    vec3 F    = fresnelSchlick(max(dot(H, viewDir), 0.0), F0);       
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;	  
    
    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, L), 0.0) + 0.0001;
    vec3 specular     = numerator / denominator;  
        
    float NdotL = max(dot(normal, L), 0.0);                
    return (kD * albedo / PI + specular) * radiance * NdotL; 
}

//TODO: add contributions from spot and textured lights

void main()
{		
    vec3 N = normalize(fs_in.tbnMatrix * vec4((texture(normalMap, fs_in.texCoord).rgb * 2.0) - 1.0, 1.0)).rgb;
    vec3 viewDir = normalize(viewPos - fs_in.vPos);

    vec3 albedo = texture(albedoMap, fs_in.texCoord).rgb;
    vec3 ao = texture(aoMap, fs_in.texCoord).rgb;
    float metallic = texture(metallicMap, fs_in.texCoord).r;
    float roughness = texture(roughnessMap, fs_in.texCoord).r;

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);
	           
    // reflectance equation
    vec3 Lo = vec3(0.0);
    // for(int i = 0; i < 4; ++i) 
    // {
    //     // calculate per-light radiance
    //     vec3 L = normalize(lightPositions[i] - fs_in.vPos);
    //     vec3 H = normalize(viewDir + L);
    //     float distance    = length(lightPositions[i] - fs_in.vPos);
    //     float attenuation = 1.0 / (distance * distance);
    //     vec3 radiance     = lightColors[i] * attenuation;        
        
    //     // cook-torrance brdf
    //     float NDF = DistributionGGX(N, H, roughness);        
    //     float G   = GeometrySmith(N, viewDir, L, roughness);      
    //     vec3 F    = fresnelSchlick(max(dot(H, viewDir), 0.0), F0);       
        
    //     vec3 kS = F;
    //     vec3 kD = vec3(1.0) - kS;
    //     kD *= 1.0 - metallic;	  
        
    //     vec3 numerator    = NDF * G * F;
    //     float denominator = 4.0 * max(dot(N, viewDir), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    //     vec3 specular     = numerator / denominator;  
            
    //     // add to outgoing radiance Lo
    //     float NdotL = max(dot(N, L), 0.0);                
    //     Lo += (kD * albedo / PI + specular) * radiance * NdotL; 
    // }   

    // PointLight light, int lightIdx, vec3 normal, vec3 viewDir, vec3 albedo, vec3 F0, float metallic, float roughness, vec3 fPos

    for (int d = 0; d < numDirectionalLightsBound; ++d)
    {
        Lo += CalculateDirectionalLightRadianceContribution(dirLights[d], d, N, viewDir, albedo, F0, metallic, roughness, fs_in.vPos);
    }

    for (int p = 0; p < numPointLightsBound; ++p)
    {
        Lo += CalculatePointLightRadianceContribution(pointLights[p], N, viewDir, albedo, F0, metallic, roughness, fs_in.vPos);
    }
  
    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 color = ambient + Lo;  
   
    FragColor = vec4(color, 1.0);
}