#version 460

uniform sampler2D texColor;
uniform sampler2D texDepth;

uniform int viewportWidth;
uniform int viewportHeight;

layout (location = 0) out vec4 finalColor;
layout (depth_greater) out float gl_FragDepth;

void main()
{
    vec2 uv = gl_FragCoord.xy / vec2(viewportWidth, viewportHeight);
    ivec2 realFragmentCoord = ivec2(uv * vec2(1920, 1080));

    gl_FragDepth = texelFetch(texDepth, realFragmentCoord, 0).r;

    vec4 col = texelFetch(texColor, realFragmentCoord, 0);
    finalColor = col;
}
