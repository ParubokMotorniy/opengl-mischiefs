#version 460 core

// uvec3	gl_NumWorkGroups	    number of work groups that have been dispatched set by glDispatchCompute()
// uvec3	gl_WorkGroupSize	    size of the work group (local size) operated on defined with layout
// uvec3	gl_WorkGroupID	        index of the work group currently being operated on
// uvec3	gl_LocalInvocationID	index of the current work item in the work group
// uvec3	gl_GlobalInvocationID	global index of the current work item (gl_WorkGroupID * gl_WorkGroupSize + gl_LocalInvocationID)
// uint	    gl_LocalInvocationIndex	1d index representation of gl_LocalInvocationID (gl_LocalInvocationID.z * gl_WorkGroupSize.x * gl_WorkGroupSize.y + gl_LocalInvocationID.y * gl_WorkGroupSize.x + gl_LocalInvocationID.x)

//TODO: before I do anything more complex, the current implementation has to be optimized (it definitely can be):
// 1. ~Double buffering~
// 2. ~Render only depth. No need to keep a large view position texture around~
// 3. Optimize the marching itself. Can splitting it into multiple passes help?
// 4. ~Adaptive step size. I can't really use SFD, but I can chabge the step size based on the distance of the sphere to the camera~
// 4. Profit?

//TODO: move to 32-bit depth
//TODO: optimize the light marching

//TODO: how to further paralelize
// 1. First pass computes desnity+transmittance at sample points
// 2. Second pass dispatches as many threads as there are sample points to compute shadowing at each point (over all lights. This dimension can alos be splitted but that would entail even more synchronization within warps what can deteriorate the performance). Along the ray, the finalight's are summed into an atomic color value.
// 3. Third pass determines the final color of the fragment
// 4. Profit? This does not seem to be a task for a day before the presentation. Way too much places where things can go wrong. So I have to come up with something else. Possibly dispatch more threads for a single invocation.

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
layout(r16f, binding = 1) uniform image2D depthOutputImage;

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

uniform vec3 shadowColor;
uniform vec3 fogColor;
uniform float transmittance;
uniform float darknessThreshold;
uniform float lightAbsorb;

uniform mat3 fogVolumeWorldToModelRotation;

uniform sampler3D fogTexture;

const float sqrt2 = 1.414213562;
const float lightDensityThreshold = 0.01;
const float PI = 3.141592654;
const float DUAL_LOBE_WEIGHT = 0.7; //TODO: the fuck this parameter takes care of?

float computeDensityContributionWithinTexture(vec3 rayPosition, float inscribedRadius)
{
    vec3 localFogVector = fogVolumeWorldToModelRotation * mat3(inverseViewMatrix) * ((rayPosition - viewSpherePos) / inscribedRadius);

    localFogVector += 0.5;

    float densityFloor = textureLod(fogTexture, localFogVector, currentMipLevelFloor).a;
    float densityCeiling = textureLod(fogTexture, localFogVector, currentMipLevelCeiling).a;

    return mix(densityFloor, densityCeiling, lodMixingCoefficient);
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

vec3 saturate3(vec3 x) {
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

// float IsotropicPhaseFunction(float g, float costh)
// {
//     return 1.0 / (4.0 * PI);
// }

float DualHenyeyGreenstein(float g, float costh)
{
    return mix(HenyeyGreenstein(-g, costh), HenyeyGreenstein(g, costh), DUAL_LOBE_WEIGHT);
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
        float beers = exp(-density * lightAbsorb * a); //TODO: maybe, make beers multichanneled

        luminance += b * phaseFunction * beers;

        a *= attenuation;
        b *= contribution;
        c *= (1.0 - phaseAttenuation);
    }
    return luminance;
}

vec3 CalculateLightEnergy(
    vec3 lightOrigin,
    vec3 lightDirection,
    float mu,
    float stepLength,
    float maxLightSteps,
    float inscribedRadius
)
{
    float lightRayDensity = 0.0;
    float distanceMarchedToLight = 0.0;

    for(float j = 0.0; j < maxLightSteps; j++)
    {
        vec3 lightSamplePos = lightOrigin + lightDirection * distanceMarchedToLight;

        lightRayDensity += computeDensityContributionWithinTexture(lightSamplePos, inscribedRadius) * stepLength;
        distanceMarchedToLight += stepLength;
    }

    vec3 beersLaw = MultipleOctaveScattering(lightRayDensity, mu);
    float powder = 1.0 - exp(-lightRayDensity * 2.0 * lightAbsorb);

    return beersLaw * mix(2.0 * vec3(powder, powder, powder), vec3(1.0), remap(mu, -1.0, 1.0, 0.0, 1.0));
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
    const float minDistanceToSphere = abs(viewSpherePos.z) - sphereRadius;
    const float actualMaxMarchDistance = min(maxMarchDistance, abs(viewSpherePos.z) + sphereRadius);
    const float inscribedRadius = sphereRadius / sqrt2;
    const float maxLightMarchDistance = 2.0 * sphereRadius;

    //marches the ray

    float densityAccumulation = 0.0;
    float distanceMarched = minDistanceToSphere;
    float fragmentDepth = 1.0;

    vec3 luminance = vec3(0.0);
    float finalLight = 0;
    float transmittance = 0.01;

    ScatteringTransmittance marchedFogParameters;
    marchedFogParameters.scattering = vec3(0.0, 0.0, 0.0);
    marchedFogParameters.transmittance = vec3(1.0, 1.0, 1.0);

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

        densityAccumulation += currentDensityContribution;

        if(currentDensityContribution < lightDensityThreshold)
            continue;

        //directional light effect
        for(int d = 0; d < numDirectionalLightsBound; ++d)
        {
            vec3 lightDirection = normalize(mat3(viewMatrix) * (-dirLights[d].direction)); //the light direction also has to be transformed to view space, with no translation
            float mu = dot(rayDirection, lightDirection);
            vec3 srcLuminance = dirLights[d].diffuse * CalculateLightEnergy(rayPosition, lightDirection, mu, marchStepSize, maxLightMarchDistance / marchStepSize, inscribedRadius);

            luminance += srcLuminance;
        }

        //point light effect
        for(int p = 0; p < numPointLightsBound; ++p)
        {
            vec3 lightViewPosition = (viewMatrix * vec4(pointLights[p].position, 1.0)).xyz;
            vec3 lightDirection = normalize(lightViewPosition - rayPosition);

            float mu = dot(rayDirection, lightDirection);
            vec3 srcLuminance = pointLights[p].diffuse * CalculateLightEnergy(rayPosition, lightDirection, mu, marchStepSize, maxLightMarchDistance / marchStepSize, inscribedRadius);

            luminance += srcLuminance;
        }

        //spot light effect
        for(int s = 0; s < numSpotLightsBound; ++s)
        {
            SpotLight testedLight = spotLights[s];
            vec3 lightViewPosition = (viewMatrix * vec4(testedLight.position, 1.0)).xyz;
            vec3 lightDirection = normalize(lightViewPosition - rayPosition);

            float mu = dot(rayDirection, lightDirection);
            vec3 srcLuminance = testedLight.diffuse * CalculateLightEnergy(rayPosition, lightDirection, mu, marchStepSize, maxLightMarchDistance / marchStepSize, inscribedRadius);

            luminance += srcLuminance;
        }

        float transmittance = exp(-currentDensityContribution * marchStepSize * lightAbsorb);
        vec3 integScatt = currentDensityContribution * (luminance - luminance * transmittance) / currentDensityContribution;

        marchedFogParameters.scattering += marchedFogParameters.transmittance * integScatt;
        marchedFogParameters.transmittance *= transmittance;

        if(length(marchedFogParameters.transmittance) <= 0.01)
        {
            marchedFogParameters.transmittance = vec3(0.0);
            break;
        }
    }

    vec3 scatteredFogColor = vec3(marchedFogParameters.scattering) * fogColor;
    float meanScattering = max(mean(marchedFogParameters.scattering), 1.0);

    marchedFogParameters.transmittance = saturate3(marchedFogParameters.transmittance);

    vec3 colour =  marchedFogParameters.transmittance + scatteredFogColor * 1.0;// * CLOUD_EXPOSURE;

    imageStore(depthOutputImage, imgCoords, vec4(fragmentDepth, fragmentDepth, fragmentDepth, fragmentDepth));
    imageStore(colorOutput, imgCoords, vec4(colour, 1.0 - exp(-densityAccumulation * meanScattering)));
};
