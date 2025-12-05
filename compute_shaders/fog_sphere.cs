#version 460 core

// uvec3	gl_NumWorkGroups	    number of work groups that have been dispatched set by glDispatchCompute()
// uvec3	gl_WorkGroupSize	    size of the work group (local size) operated on defined with layout
// uvec3	gl_WorkGroupID	        index of the work group currently being operated on
// uvec3	gl_LocalInvocationID	index of the current work item in the work group
// uvec3	gl_GlobalInvocationID	global index of the current work item (gl_WorkGroupID * gl_WorkGroupSize + gl_LocalInvocationID)
// uint	    gl_LocalInvocationIndex	1d index representation of gl_LocalInvocationID (gl_LocalInvocationID.z * gl_WorkGroupSize.x * gl_WorkGroupSize.y + gl_LocalInvocationID.y * gl_WorkGroupSize.x + gl_LocalInvocationID.x)

layout (local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout(rgba16f, binding = 0) uniform image2D colorOutput;
layout(rgba16f, binding = 1) uniform image2D viewSpacePosOutput;

uniform int resolutionX;
uniform int resolutionY;

uniform vec3  spherePos;
uniform float sphereRadius;

uniform mat4  viewMatrix;
uniform mat4 clipToView;

uniform float densityScale;

const float marchStepSize = 0.1;
const float maxMarchDistance = 100;
const float densityIncrement = 0.005;

void main()
{
    //initializes the ray

    const ivec2 imgCoords = ivec2(gl_GlobalInvocationID.xy);

    const vec3 ndcEndpoint = vec3((vec2(imgCoords) / vec2(resolutionX, resolutionY)) * 2.0 - 1.0, 1.0);
    const vec4 viewEndpoint = clipToView * vec4(ndcEndpoint, 1.0);
    const vec3 rayDirection = normalize(viewEndpoint.xyz / viewEndpoint.w);

    const vec3 viewSpherePos = (viewMatrix * vec4(spherePos, 1.0)).xyz;
    const float radiusSquared = sphereRadius * sphereRadius;

    //marches the ray

    float densityAccumulation = 0.0;
    float distanceMarched = 0.0;
    bool sphereEntered = false;

    while(distanceMarched < maxMarchDistance)
    {
        vec3 rayPosition = rayDirection * distanceMarched;
        if(dot(rayPosition - viewSpherePos,rayPosition - viewSpherePos) <= radiusSquared)
        {
            densityAccumulation += densityIncrement;

            if(!sphereEntered)
            {
                sphereEntered = true;
                imageStore(viewSpacePosOutput, imgCoords, vec4(rayPosition, 0.0));
            }
        }
        distanceMarched += marchStepSize;
    }

    densityAccumulation *= densityScale;
    imageStore(colorOutput, imgCoords, vec4(densityAccumulation, densityAccumulation, densityAccumulation, densityAccumulation));
};
