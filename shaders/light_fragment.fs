#version 400 core

uniform vec3 actualLightColor;
out vec4 fragColor;

void main()
{
    fragColor = vec4(actualLightColor, 1.0f);
}
