#version 460 core

// uvec3	gl_NumWorkGroups	    number of work groups that have been dispatched set by glDispatchCompute()
// uvec3	gl_WorkGroupSize	    size of the work group (local size) operated on defined with layout
// uvec3	gl_WorkGroupID	        index of the work group currently being operated on
// uvec3	gl_LocalInvocationID	index of the current work item in the work group
// uvec3	gl_GlobalInvocationID	global index of the current work item (gl_WorkGroupID * gl_WorkGroupSize + gl_LocalInvocationID)
// uint	    gl_LocalInvocationIndex	1d index representation of gl_LocalInvocationID (gl_LocalInvocationID.z * gl_WorkGroupSize.x * gl_WorkGroupSize.y + gl_LocalInvocationID.y * gl_WorkGroupSize.x + gl_LocalInvocationID.x)

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(rgba32f, binding = 0) uniform image2D colorOutput;
layout(rgba32f, binding = 1) uniform image2D viewSpacePosOutput;

uniform float aspectRatio; //width / height
uniform float nearClip;
uniform int resolutionX;
uniform int resolutionY;

uniform vec3  spherePos;
uniform float sphereRadius;
uniform mat4  viewMatrix;

const float marchStepSize = 0.001;
const float maxMarchDistance = 100;
const float densityIncrement = 0.01;

void main()
{
    //initializes the ray

    const ivec2 imgCoords = ivec2(gl_GlobalInvocationID.xy);
    const vec3 rayDirection = vec3((imgCoords / uvec2(resolutionX, resolutionY)) * vec2(aspectRatio, 1.0), nearClip);
    
    const vec3 viewSpherePos = (viewMatrix * vec4(spherePos, 1.0)).xyz;
    const float radiusSquared = sphereRadius * sphereRadius;

    //marches the ray

    float densityAccumulation = 0.0;
    float distanceMarched = 0.0;

    while(distanceMarched < maxMarchDistance)
    {
        vec3 rayPosition = rayDirection * distanceMarched;
        if(dot(rayPosition - viewSpherePos, rayPosition - viewSpherePos) <= radiusSquared)
        {
            imageStore(viewSpacePosOutput, imgCoords, vec4(rayPosition, 0.0)); //the first point of contact will give fragment depth in the future
            break;
        }
        distanceMarched += marchStepSize;
    }   

    while(distanceMarched < maxMarchDistance)
    {
        vec3 rayPosition = rayDirection * distanceMarched;
        if(dot(rayPosition - viewSpherePos,rayPosition - viewSpherePos) <= radiusSquared)
        {
            densityAccumulation += densityIncrement;
            distanceMarched += marchStepSize;
        }else
        {
            break; //the ray has left the sphere
        }
    }

    imageStore(colorOutput, imgCoords, vec4(densityAccumulation, densityAccumulation, densityAccumulation, densityAccumulation));
};
