#version 460 core

// uvec3	gl_NumWorkGroups	    number of work groups that have been dispatched set by glDispatchCompute()
// uvec3	gl_WorkGroupSize	    size of the work group (local size) operated on defined with layout
// uvec3	gl_WorkGroupID	        index of the work group currently being operated on
// uvec3	gl_LocalInvocationID	index of the current work item in the work group
// uvec3	gl_GlobalInvocationID	global index of the current work item (gl_WorkGroupID * gl_WorkGroupSize + gl_LocalInvocationID)
// uint	    gl_LocalInvocationIndex	1d index representation of gl_LocalInvocationID (gl_LocalInvocationID.z * gl_WorkGroupSize.x * gl_WorkGroupSize.y + gl_LocalInvocationID.y * gl_WorkGroupSize.x + gl_LocalInvocationID.x)

layout(local_size_x = 32, local_size_y = 16, local_size_z = 1) in;

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

#define NUM_DIRECTIONAL 8
layout(binding = 1, std140) uniform DirectionalLights
{
    DirectionalLight dirLights[NUM_DIRECTIONAL];
};
uniform int numDirectionalLightsBound;

#define NUM_POINT 8
layout(binding = 2, std140) uniform PointLights
{
    PointLight pointLights[NUM_POINT];
};
uniform int numPointLightsBound;

#define NUM_SPOT 8
layout(binding = 3, std140) uniform SpotLights
{
    SpotLight spotLights[NUM_SPOT];
};
uniform int numSpotLightsBound;

layout(rgba16f, binding = 0) uniform image2D colorOutput;
layout(r32f, binding = 1) uniform image2D depthOutputImage;

uniform int resolutionX;
uniform int resolutionY;
uniform int currentMipLevelFloor;
uniform int currentMipLevelCeiling;
uniform float lodMixingCoefficient;
uniform float marchStepSize;
uniform float maxMarchDistance;

uniform vec3 viewSpherePos;
uniform float sphereRadius;

uniform mat4 viewMatrix;
uniform mat4 inverseViewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 clipToView;

uniform float densityScale;

uniform vec3 fogColor;
uniform float transmittance;
uniform vec3 lightAbsorb;
uniform float noiseInfluence;

uniform mat3 fogVolumeWorldToModelRotation;

uniform sampler3D fogTexture;

const float sqrt2 = 1.414213562;
const float lightDensityThreshold = 0.01;
const float PI = 3.141592654;

float Perlin3D(vec3 P)
{
    //  https://github.com/BrianSharpe/Wombat/blob/master/Perlin3D.glsl

    // establish our grid cell and unit position
    vec3 Pi = floor(P);
    vec3 Pf = P - Pi;
    vec3 Pf_min1 = Pf - 1.0;

    // clamp the domain
    Pi.xyz = Pi.xyz - floor(Pi.xyz * (1.0 / 69.0)) * 69.0;
    vec3 Pi_inc1 = step(Pi, vec3(69.0 - 1.5)) * (Pi + 1.0);

    // calculate the hash
    vec4 Pt = vec4(Pi.xy, Pi_inc1.xy) + vec2(50.0, 161.0).xyxy;
    Pt *= Pt;
    Pt = Pt.xzxz * Pt.yyww;
    const vec3 SOMELARGEFLOATS = vec3(635.298681, 682.357502, 668.926525);
    const vec3 ZINC = vec3(48.500388, 65.294118, 63.934599);
    vec3 lowz_mod = vec3(1.0 / (SOMELARGEFLOATS + Pi.zzz * ZINC));
    vec3 highz_mod = vec3(1.0 / (SOMELARGEFLOATS + Pi_inc1.zzz * ZINC));
    vec4 hashx0 = fract(Pt * lowz_mod.xxxx);
    vec4 hashx1 = fract(Pt * highz_mod.xxxx);
    vec4 hashy0 = fract(Pt * lowz_mod.yyyy);
    vec4 hashy1 = fract(Pt * highz_mod.yyyy);
    vec4 hashz0 = fract(Pt * lowz_mod.zzzz);
    vec4 hashz1 = fract(Pt * highz_mod.zzzz);

    // calculate the gradients
    vec4 grad_x0 = hashx0 - 0.49999;
    vec4 grad_y0 = hashy0 - 0.49999;
    vec4 grad_z0 = hashz0 - 0.49999;
    vec4 grad_x1 = hashx1 - 0.49999;
    vec4 grad_y1 = hashy1 - 0.49999;
    vec4 grad_z1 = hashz1 - 0.49999;
    vec4 grad_results_0 = inversesqrt(grad_x0 * grad_x0 + grad_y0 * grad_y0 + grad_z0 * grad_z0) * (vec2(Pf.x, Pf_min1.x).xyxy * grad_x0 + vec2(Pf.y, Pf_min1.y).xxyy * grad_y0 + Pf.zzzz * grad_z0);
    vec4 grad_results_1 = inversesqrt(grad_x1 * grad_x1 + grad_y1 * grad_y1 + grad_z1 * grad_z1) * (vec2(Pf.x, Pf_min1.x).xyxy * grad_x1 + vec2(Pf.y, Pf_min1.y).xxyy * grad_y1 + Pf_min1.zzzz * grad_z1);

    // Classic Perlin Interpolation
    vec3 blend = Pf * Pf * Pf * (Pf * (Pf * 6.0 - 15.0) + 10.0);
    vec4 res0 = mix(grad_results_0, grad_results_1, blend.z);
    vec4 blend2 = vec4(blend.xy, vec2(1.0 - blend.xy));
    float final = dot(res0, blend2.zxzx * blend2.wwyy);
    return (final * 1.1547005383792515290182975610039);  // scale things to a strict -1.0->1.0 range  *= 1.0/sqrt(0.75)
}

float Value3D(vec3 P)
{
    //  https://github.com/BrianSharpe/Wombat/blob/master/Value3D.glsl

    // establish our grid cell and unit position
    vec3 Pi = floor(P);
    vec3 Pf = P - Pi;
    vec3 Pf_min1 = Pf - 1.0;

    // clamp the domain
    Pi.xyz = Pi.xyz - floor(Pi.xyz * (1.0 / 69.0)) * 69.0;
    vec3 Pi_inc1 = step(Pi, vec3(69.0 - 1.5)) * (Pi + 1.0);

    // calculate the hash
    vec4 Pt = vec4(Pi.xy, Pi_inc1.xy) + vec2(50.0, 161.0).xyxy;
    Pt *= Pt;
    Pt = Pt.xzxz * Pt.yyww;
    vec2 hash_mod = vec2(1.0 / (635.298681 + vec2(Pi.z, Pi_inc1.z) * 48.500388));
    vec4 hash_lowz = fract(Pt * hash_mod.xxxx);
    vec4 hash_highz = fract(Pt * hash_mod.yyyy);

    //	blend the results and return
    vec3 blend = Pf * Pf * Pf * (Pf * (Pf * 6.0 - 15.0) + 10.0);
    vec4 res0 = mix(hash_lowz, hash_highz, blend.z);
    vec4 blend2 = vec4(blend.xy, vec2(1.0 - blend.xy));
    return dot(res0, blend2.zxzx * blend2.wwyy);
}

float computeDensityContributionWithinTexture(vec3 rayPosition, float inscribedRadius)
{
    vec3 localFogVector = fogVolumeWorldToModelRotation * mat3(inverseViewMatrix) * ((rayPosition - viewSpherePos) / inscribedRadius);

    localFogVector += 0.5;

    vec3 unnormalizedLocalFogVector = fogVolumeWorldToModelRotation * mat3(inverseViewMatrix) * (rayPosition - viewSpherePos);
    float coeff = max(Perlin3D(unnormalizedLocalFogVector) * Value3D(unnormalizedLocalFogVector), 1.0 - noiseInfluence);

    //trilinear filtering
    float densityFloor = textureLod(fogTexture, localFogVector, currentMipLevelFloor).a;
    float densityCeiling = textureLod(fogTexture, localFogVector, currentMipLevelCeiling).a;
    return mix(densityFloor, densityCeiling, lodMixingCoefficient) * coeff;
}

float inverseLerp(float minValue, float maxValue, float v)
{
    return (v - minValue) / (maxValue - minValue);
}

float remap(float v, float inMin, float inMax, float outMin, float outMax)
{
    float t = inverseLerp(inMin, inMax, v);
    return mix(outMin, outMax, t);
}

vec3 saturate3(vec3 x)
{
    return clamp(x, vec3(0.0), vec3(1.0));
}

float mean(vec3 x)
{
    return (x.x + x.y + x.z) / 3.0;
}

struct ScatteringTransmittance
{
    vec3 scattering;
    vec3 transmittance;
};

float HenyeyGreenstein(float g, float mu)
{
    float gg = g * g;
    return (1.0 / (4.0 * PI)) * ((1.0 - gg) / pow(1.0 + gg - 2.0 * g * mu, 1.5));
}

float DualHenyeyGreenstein(float g, float costh)
{
    return mix(HenyeyGreenstein(-g, costh), HenyeyGreenstein(g, costh), 0.7);
}

float PhaseFunction(float g, float costh)
{
    return DualHenyeyGreenstein(g, costh);
}

vec3 MultipleOctaveScattering(float density, float mu)
{
    float attenuation = 0.2;
    float contribution = 0.2;
    float phaseAttenuation = 0.5;

    float a = 1.0;
    float b = 1.0;
    float c = 1.0;
    float g = 0.85;
    const float scatteringOctaves = 4.0;

    vec3 luminance = vec3(0.0);

    for(float i = 0.0; i < scatteringOctaves; i++)
    {
        float phaseFunction = PhaseFunction(0.3 * c, mu);
        vec3 beers = exp(-density * lightAbsorb * a);
        // vec3 powder = 1.0 - exp(-density * 2.0 * lightAbsorb);

        luminance += b * phaseFunction * beers;

        a *= attenuation;
        b *= contribution;
        c *= (1.0 - phaseAttenuation);
    }
    return luminance;
}

//adapted and experimented with: https://github.com/simondevyoutube/Shaders_Clouds1

vec3 CalculateLightEnergy(
    vec3 lightOrigin,
    vec3 lightDirection,
    float mu,
    float stepLength,
    float maxLightSteps,
    float inscribedRadius,
    float sphereRadiusSquared
)
{
    float lightRayDensity = 0.0;
    float distanceMarchedToLight = 0.0;

    for(float j = 0.0; j < maxLightSteps; j++)
    {
        vec3 lightSamplePos = lightOrigin + lightDirection * distanceMarchedToLight;

        if(dot(lightSamplePos - viewSpherePos, lightSamplePos - viewSpherePos) <= sphereRadiusSquared)
            break;

        lightRayDensity += computeDensityContributionWithinTexture(lightSamplePos, inscribedRadius) * stepLength;
        distanceMarchedToLight += stepLength;
    }

    vec3 beersLaw = MultipleOctaveScattering(lightRayDensity, mu);
    vec3 powder = 1.0 - exp(-lightRayDensity * 2.0 * lightAbsorb);

    return beersLaw * mix(2.0 * powder, vec3(1.0), remap(mu, -1.0, 1.0, 0.0, 1.0));
    // return beersLaw;
}

void main()
{
    //initializes the ray

    const ivec2 imgCoords = ivec2(gl_GlobalInvocationID.xy);

    //// per ray
    const vec3 ndcEndpoint = vec3((vec2(imgCoords) / vec2(resolutionX, resolutionY)) * 2.0 - 1.0, 1.0);
    const vec4 viewEndpoint = clipToView * vec4(ndcEndpoint, 1.0);
    const vec3 rayDirection = normalize(viewEndpoint.xyz / viewEndpoint.w);

    //// shared
    const float radiusSquared = sphereRadius * sphereRadius;
    const float inscribedRadius = sphereRadius / sqrt2;

    const float minDistanceToSphere = abs(viewSpherePos.z) - sphereRadius;
    const float actualMaxMarchDistance = min(maxMarchDistance, abs(viewSpherePos.z) + sphereRadius);
    const float maxLightMarchDistance = 2.0 * sphereRadius;

    //marches the ray

    float densityAccumulation = 0.0;
    float distanceMarched = minDistanceToSphere;
    float fragmentDepth = 1.0;

    vec3 luminance = vec3(0.0);

    ScatteringTransmittance marchedFogParameters;
    marchedFogParameters.scattering = vec3(0.0, 0.0, 0.0);
    marchedFogParameters.transmittance = vec3(transmittance, transmittance, transmittance);

    while(distanceMarched < actualMaxMarchDistance)
    {
        vec3 rayPosition = rayDirection * distanceMarched;
        distanceMarched += marchStepSize;
        bool inSphere = dot(rayPosition - viewSpherePos, rayPosition - viewSpherePos) <= radiusSquared;

        if(!inSphere)
            continue;

        vec4 clip = projectionMatrix * vec4(rayPosition, 1.0);
        float depth = clip.z / clip.w;
        depth = depth * 0.5 + 0.5;
        fragmentDepth = min(fragmentDepth, depth);
        const float currentDensityContribution = computeDensityContributionWithinTexture(rayPosition, inscribedRadius) * densityScale;

        densityAccumulation += currentDensityContribution * marchStepSize;

        if(currentDensityContribution < lightDensityThreshold)
            continue;

        //directional light effect
        for(int d = 0; d < numDirectionalLightsBound; ++d)
        {
            vec3 lightDirection = normalize(mat3(viewMatrix) * (-dirLights[d].direction)); //the light direction also has to be transformed to view space, with no translation
            float mu = dot(rayDirection, lightDirection);
            vec3 srcLuminance = dirLights[d].diffuse * CalculateLightEnergy(rayPosition, lightDirection, mu, marchStepSize, maxLightMarchDistance / marchStepSize, inscribedRadius, radiusSquared);

            luminance += srcLuminance;
        }

        //point light effect
        for(int p = 0; p < numPointLightsBound; ++p)
        {
            PointLight testedLight = pointLights[p];
            vec3 lightViewPosition = (viewMatrix * vec4(testedLight.position, 1.0)).xyz;
            float distToLight = length(lightViewPosition - rayPosition);

            float attenuation = clamp(1.0 / (testedLight.attenuationConstantTerm + testedLight.attenuationLinearTerm * distToLight + testedLight.attenuationQuadraticTerm * (distToLight * distToLight)), 0.0, 1.0);

            if(attenuation <= 0.0)
                continue;

            vec3 lightDirection = normalize(lightViewPosition - rayPosition);
            float mu = dot(rayDirection, lightDirection);
            vec3 srcLuminance = pointLights[p].diffuse * CalculateLightEnergy(rayPosition, lightDirection, mu, marchStepSize, maxLightMarchDistance / marchStepSize, inscribedRadius, radiusSquared);

            luminance += srcLuminance;
        }

        //spot light effect
        for(int s = 0; s < numSpotLightsBound; ++s)
        {
            SpotLight testedLight = spotLights[s];
            vec3 lightViewPosition = (viewMatrix * vec4(testedLight.position, 1.0)).xyz;
            float distToLight = length(lightViewPosition - rayPosition);

            float attenuation = clamp(1.0 / (testedLight.attenuationConstantTerm + testedLight.attenuationLinearTerm * distToLight + testedLight.attenuationQuadraticTerm * (distToLight * distToLight)), 0.0, 1.0);

            if(attenuation <= 0.0)
                continue;

            vec3 lightDirection = normalize(lightViewPosition - rayPosition);
            float mu = dot(rayDirection, lightDirection);
            vec3 srcLuminance = testedLight.diffuse * CalculateLightEnergy(rayPosition, lightDirection, mu, marchStepSize, maxLightMarchDistance / marchStepSize, inscribedRadius, radiusSquared);

            luminance += srcLuminance;
        }

        vec3 transmittance = exp(-currentDensityContribution * marchStepSize * lightAbsorb);
        vec3 integScatt = currentDensityContribution * (luminance - luminance * transmittance) / currentDensityContribution;

        marchedFogParameters.scattering += marchedFogParameters.transmittance * integScatt;
        marchedFogParameters.transmittance *= transmittance;

        if(length(marchedFogParameters.transmittance) <= 0.01)
        {
            marchedFogParameters.transmittance = vec3(0.0);
            break;
        }
    }

    marchedFogParameters.transmittance = saturate3(marchedFogParameters.transmittance);

    vec3 scatteredFogColor = vec3(marchedFogParameters.scattering) * fogColor;

    float meanTransmittance = max(mean(marchedFogParameters.scattering), 1.0);

    //TODO: ideally, I should belnd with the background color basing on the transmittance. But the following also seems to give nice results

    imageStore(depthOutputImage, imgCoords, vec4(fragmentDepth, fragmentDepth, fragmentDepth, fragmentDepth));
    imageStore(colorOutput, imgCoords, vec4(scatteredFogColor, exp(-meanTransmittance) * (1.0 - exp(-densityAccumulation))));
};
