#version 460

uniform sampler2D texColor;
uniform sampler2D texPosition;
uniform mat4 projectionMatrix;

uniform int viewportWidth;
uniform int viewportHeight;

layout (location = 0) out vec4 finalColor;

void main()
{
    vec2 uv = gl_FragCoord.xy / vec2(viewportWidth, viewportHeight);
    ivec2 realFragmentCoord = ivec2(uv * vec2(1920, 1080));

    vec3 viewPos = texelFetch(texPosition, realFragmentCoord, 0).xyz;

    vec4 clip = projectionMatrix * vec4(viewPos, 1.0);
    float depth = clip.z / clip.w;
    depth = depth * 0.5 + 0.5;
    gl_FragDepth = depth;

    vec4 col = texelFetch(texColor, realFragmentCoord, 0);
    finalColor = col;
}
