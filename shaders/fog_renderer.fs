#version 460

uniform sampler2D texColor;
uniform sampler2D texPosition;
uniform mat4 projection;

layout (location = 0) out vec4 finalColor;

void main()
{
    vec3 viewPos = texelFetch(texPosition, ivec2(gl_FragCoord.xy), 0).xyz;

    vec4 clip = projection * vec4(viewPos, 1.0);
    float depth = clip.z / clip.w;
    depth = depth * 0.5 + 0.5;

    gl_FragDepth = depth;

    vec4 col = texelFetch(texColor, ivec2(gl_FragCoord.xy), 0);
    finalColor = col;
}
