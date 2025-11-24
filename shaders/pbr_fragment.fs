#version 460 core
#extension GL_ARB_bindless_texture : require

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
    mat3 tbnMatrix;
} fs_in;
  
float DistributionGGX(vec3 fNormal, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(fNormal, H), 0.0);
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
float GeometrySmith(vec3 fNormal, vec3 viewDir, vec3 L, float roughness)
{
    float NdotV = max(dot(fNormal, viewDir), 0.0);
    float NdotL = max(dot(fNormal, L), 0.0);
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
    float shadowEffect = 1.0 - fragmentInDirectionalShadow(light, lightIdx, fPos, normal);
    vec3 radiance     = light.diffuse * shadowEffect;  
    
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

vec3 CalculateSpotLightRadianceContribution(SpotLight light, vec3 normal, vec3 viewDir, vec3 albedo, vec3 F0, float metallic, float roughness, vec3 fPos)
{
    vec3 L = normalize(light.position - fPos);
    vec3 H = normalize(viewDir + L);
    float distance = length(light.position - fPos);
    float attenuation = clamp(1.0
                                  / (light.attenuationConstantTerm
                                     + light.attenuationLinearTerm * distance
                                     + light.attenuationQuadraticTerm * (distance * distance)),
                              0.0, 1.0);
    float theta = dot(L, normalize(-light.direction));
    float epsilon = (light.innerCutOff - light.outerCutOff);
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    vec3 radiance     = light.diffuse * attenuation * intensity;      
    
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

vec3 CalculateTexturedSpotLightRadianceContribution(TexturedSpotLight light, vec3 normal, vec3 viewDir, vec3 albedo, vec3 F0, float metallic, float roughness, vec3 fPos)
{
    vec3 L = normalize(light.position - fPos);
    vec3 H = normalize(viewDir + L);
    float distance = length(light.position - fPos);

    //projected texture sampling
    vec4 mappedFrag = light.lightProj * light.lightView * vec4(fPos, 1.0f);
    mappedFrag /= mappedFrag.w;
    float tCoordX = (mappedFrag.x + 1.0) / 2.0f;
    float tCoordY = (-mappedFrag.y + 1.0) / 2.0f;
    vec3 lightDiffuse = texture(sampler2D(light.textureIdx), vec2(tCoordX, tCoordY)).xyz;

    float attenuation = clamp(1.0
                                  / (light.attenuationConstantTerm
                                     + light.attenuationLinearTerm * distance
                                     + light.attenuationQuadraticTerm * (distance * distance)),
                              0.0, 1.0);
    float theta = dot(L, normalize(-light.direction));
    float epsilon = (light.innerCutOff - light.outerCutOff);
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    vec3 radiance     = lightDiffuse * attenuation * intensity;      
    
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

void main()
{		
    vec3 fNormal = normalize(fs_in.tbnMatrix * ((texture(normalMap, fs_in.texCoord).rgb * 2.0) - 1.0)).rgb;
    vec3 viewDir = normalize(viewPos - fs_in.vPos);

    vec3 albedo = texture(albedoMap, fs_in.texCoord).rgb;
    float ao = texture(aoMap, fs_in.texCoord).r;
    float metallic = texture(metallicMap, fs_in.texCoord).r;
    float roughness = texture(roughnessMap, fs_in.texCoord).r;

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);
	           
    // reflectance equation
    vec3 Lo = vec3(0.0);
    for (int d = 0; d < numDirectionalLightsBound; ++d)
    {
        Lo += CalculateDirectionalLightRadianceContribution(dirLights[d], d, fNormal, viewDir, albedo, F0, metallic, roughness, fs_in.vPos);
    }

    for (int p = 0; p < numPointLightsBound; ++p)
    {
        Lo += CalculatePointLightRadianceContribution(pointLights[p], fNormal, viewDir, albedo, F0, metallic, roughness, fs_in.vPos);
    }

    for (int p = 0; p < numSpotLightsBound; ++p)
    {
        Lo += CalculateSpotLightRadianceContribution(spotLights[p], fNormal, viewDir, albedo, F0, metallic, roughness, fs_in.vPos);
    }

    for (int p = 0; p < numTexturedLightsBound; ++p)
    {
        Lo += CalculateTexturedSpotLightRadianceContribution(texturedSpotLights[p], fNormal, viewDir, albedo, F0, metallic, roughness, fs_in.vPos);
    }
  
    vec3 ambient = vec3(0.05) * albedo * ao;
    vec3 color = Lo + ambient;  
   
    FragColor = vec4(color, 1.0);
}