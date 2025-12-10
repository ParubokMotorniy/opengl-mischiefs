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

//TODO: sample fog color from texture 

layout (local_size_x = 32, local_size_y = 16, local_size_z = 1) in;

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
layout(binding = 1, std140) uniform DirectionalLights{ DirectionalLight dirLights[NUM_DIRECTIONAL]; };
uniform int numDirectionalLightsBound;

#define NUM_POINT 8
layout(binding = 2, std140) uniform PointLights { PointLight pointLights[NUM_POINT]; };
uniform int numPointLightsBound;

#define NUM_SPOT 8
layout(binding = 3, std140) uniform SpotLights { SpotLight spotLights[NUM_SPOT]; };
uniform int numSpotLightsBound;

layout(rgba16f, binding = 0) uniform image2D colorOutput;
layout(r16f, binding = 1) uniform image2D depthOutputImage;

uniform int resolutionX;
uniform int resolutionY;
uniform int numMipLeves;
uniform int stepsPerVolume;

uniform vec3  spherePos;
uniform float sphereRadius;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 clipToView;

uniform float densityScale;

uniform vec3 shadowColor;
uniform vec3 fogColor;
uniform float transmittance;
uniform float darknessThreshold; 
uniform float lightAbsorb;

uniform sampler3D fogTexture;

const float maxMarchDistance = 50;
const float densityIncrement = 0.001;
const float sqrt2 = 1.414213562;

const float maxLightMarchDistance = 10;
// const float minLightDensityContribution = 0.001;

float computeDensityContributionWithinTexture(vec3 fogCenter, vec3 rayPosition, int mipLevel, float inscribedRadius)
{
    vec3 localFogVector = (rayPosition - fogCenter) / inscribedRadius;
    localFogVector += 0.5;
    return textureLod(fogTexture, localFogVector, mipLevel).a;
}

// vec3 getFogColor(vec3 fogCenter, vec3 rayPosition)
// {
//     vec3 localFogVector = (rayPosition - fogCenter) / sphereRadius;
//     return texture(fogTexture, localFogVector).xyz;
// }

void main()
{
    //initializes the ray

    //TODO: most of these can be passed from the dispatcher

    const ivec2 imgCoords = ivec2(gl_GlobalInvocationID.xy);

    const vec3 ndcEndpoint = vec3((vec2(imgCoords) / vec2(resolutionX, resolutionY)) * 2.0 - 1.0, 1.0);
    const vec4 viewEndpoint = clipToView * vec4(ndcEndpoint, 1.0);
    const vec3 rayDirection = normalize(viewEndpoint.xyz / viewEndpoint.w);

    const vec3 viewSpherePos = (viewMatrix * vec4(spherePos, 1.0)).xyz;
    const float radiusSquared = sphereRadius * sphereRadius;

    const float minDistanceToSphere = abs(viewSpherePos.z) - sphereRadius;
    const float actualMaxMarchDistance = min(maxMarchDistance, abs(viewSpherePos.z) + sphereRadius);

    const int currentMipLevel = int(mix(0, numMipLeves, length(viewSpherePos) / maxMarchDistance)); //TODO: consider non-linear mip level selection
    const float marchStepSize = 2.0 * sphereRadius / stepsPerVolume;
    const float inscribedRadius = sphereRadius / sqrt2;
    
    //marches the ray

    float densityAccumulation = 0.0;
    float distanceMarched = minDistanceToSphere;
    float fragmentDepth = 1.0;

	float lightAccumulation = 0;
	float finalLight = 0;
    float transmittance = 0.01;

    while(distanceMarched < actualMaxMarchDistance)
    {
        vec3 rayPosition = rayDirection * distanceMarched;
        distanceMarched += marchStepSize;
        bool inSphere = dot(rayPosition - viewSpherePos,rayPosition - viewSpherePos) <= radiusSquared;

        if(!inSphere)
            continue;
    
        vec4 clip = projectionMatrix * vec4(rayPosition, 1.0);
        float depth = clip.z / clip.w;
        depth = depth * 0.5 + 0.5;
        fragmentDepth = min(fragmentDepth, depth); 
        // densityAccumulation += densityIncrement * densityScale;
        densityAccumulation += computeDensityContributionWithinTexture(viewSpherePos, rayPosition, currentMipLevel, inscribedRadius);

        //directional light effect
        for(int d = 0; d < numDirectionalLightsBound; ++d)
        {
            vec3 lightDirection = normalize(mat3(viewMatrix) * (-dirLights[d].direction)); //the light direction also has to be transformed to view space, with no translation
            for(int lightStepsTaken = 0; float(lightStepsTaken) * marchStepSize < maxLightMarchDistance; ++lightStepsTaken)
            {
                vec3 lightRayPosition = rayPosition + (lightDirection * lightStepsTaken * marchStepSize);
                bool lightRayInSphere = dot(lightRayPosition - viewSpherePos, lightRayPosition - viewSpherePos) <= radiusSquared;

                if(!lightRayInSphere)
                    break;

                // lightAccumulation += densityIncrement;
                lightAccumulation += computeDensityContributionWithinTexture(viewSpherePos, lightRayPosition, currentMipLevel, inscribedRadius);
            }
        }

        //point light effect
        for(int p = 0; p < numPointLightsBound; ++p)
        {
            vec3 lightViewPosition = (viewMatrix * vec4(pointLights[p].position, 1.0)).xyz; 
            vec3 lightDirection = normalize(lightViewPosition  - rayPosition);
            float distToLight = length(lightViewPosition - rayPosition);
            for(int lightStepsTaken = 0; float(lightStepsTaken) * marchStepSize < maxLightMarchDistance; ++lightStepsTaken)
            {
                vec3 lightRayPosition = rayPosition + (lightDirection * lightStepsTaken * marchStepSize);
                bool lightRayInSphere = dot(lightRayPosition - viewSpherePos, lightRayPosition - viewSpherePos) <= radiusSquared;

                if(!lightRayInSphere)
                    break;

                float attenuation = clamp(1.0
                        / (pointLights[p].attenuationConstantTerm + pointLights[p].attenuationLinearTerm * distToLight
                           + pointLights[p].attenuationQuadraticTerm * (distToLight * distToLight)), 0.0, 1.0);
                // lightAccumulation += mix(minLightDensityContribution, densityIncrement, 1.0 - attenuation);

                if(attenuation <= 0.0)
                    break;

                lightAccumulation += densityIncrement;
                lightAccumulation += computeDensityContributionWithinTexture(viewSpherePos, lightRayPosition, currentMipLevel, inscribedRadius);
            }
        }

        //spot light effect
        for(int s = 0; s < numSpotLightsBound; ++s)
        {
            SpotLight testedLight = spotLights[s];
            vec3 lightViewPosition = (viewMatrix * vec4(testedLight.position, 1.0)).xyz; 
            vec3 lightDirection = normalize(lightViewPosition  - rayPosition);

            float distToLight = length(lightViewPosition - rayPosition);
            vec3 rayToLight = normalize(lightViewPosition - rayPosition);

            for(int lightStepsTaken = 0; float(lightStepsTaken) * marchStepSize < maxLightMarchDistance; ++lightStepsTaken)
            {

                vec3 lightRayPosition = rayPosition + (lightDirection * lightStepsTaken * marchStepSize);
                bool lightRayInSphere = dot(lightRayPosition - viewSpherePos, lightRayPosition - viewSpherePos) <= radiusSquared;

                if(!lightRayInSphere)
                    break;

                float attenuation = clamp(1.0
                        / (testedLight.attenuationConstantTerm + testedLight.attenuationLinearTerm * distToLight
                           + testedLight.attenuationQuadraticTerm * (distToLight * distToLight)), 0.0, 1.0);

                if(attenuation <= 0.0)
                    break;

                // float theta = dot(rayToLight, normalize(-testedLight.direction));
                // float epsilon = (testedLight.innerCutOff - testedLight.outerCutOff);
                // float intensity = clamp((theta - testedLight.outerCutOff) / epsilon, 0.0, 1.0);

                // lightAccumulation += mix(minLightDensityContribution, densityIncrement, 1.0 - attenuation * intensity);

                // lightAccumulation += densityIncrement;
                lightAccumulation += computeDensityContributionWithinTexture(viewSpherePos, lightRayPosition, currentMipLevel, inscribedRadius);
            }
        }

		//The amount of light received along the ray from param rayOrigin in the direction rayDirection
        float lightTransmission = exp(-lightAccumulation);
		//shadow tends to the darkness threshold as lightAccumulation rises
		float shadow = darknessThreshold + lightTransmission * (1.0 - darknessThreshold);
		//The final light value is accumulated based on the current density, transmittance value and the calculated shadow value 
		finalLight += densityAccumulation*transmittance*shadow;
		//Initially a param its value is updated at each step by lightAbsorb, this sets the light lost by scattering
		transmittance *= exp(-densityAccumulation * lightAbsorb);
    }

	vec4 finalColor = vec4(mix(shadowColor, fogColor, finalLight), 1.0 - exp(-densityAccumulation));

    imageStore(depthOutputImage, imgCoords, vec4(fragmentDepth,fragmentDepth,fragmentDepth,fragmentDepth));
    imageStore(colorOutput, imgCoords, finalColor);
};
