#version 400 core

out vec4 fragColor;

in vec4 outColor;

uniform vec4 extColor;

void main()
{
    fragColor = normalize(outColor * extColor);
}