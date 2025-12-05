#version 460

uniform sampler2D texColor;
uniform sampler2D texPosition;
uniform mat4 projectionMatrix;

uniform int viewportOffsetX;
uniform int viewportOffsetY;

layout (location = 0) out vec4 finalColor;

void main()
{
    ivec2 realFragmentCoord = ivec2(gl_FragCoord.xy) + ivec2(viewportOffsetX, viewportOffsetY);
    vec3 viewPos = texelFetch(texPosition, realFragmentCoord, 0).xyz;

    vec4 clip = projectionMatrix * vec4(viewPos, 1.0);
    float depth = clip.z / clip.w;
    depth = depth * 0.5 + 0.5;
    gl_FragDepth = depth;

    vec4 col = texelFetch(texColor, realFragmentCoord, 0);
    finalColor = col;
}
