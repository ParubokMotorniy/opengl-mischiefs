#version 400 core

out vec4 fragColor;

in vec4 outColor;
in vec2 texCoord;

uniform vec4 extColor;

uniform sampler2D textureSampler1;
uniform sampler2D textureSampler2;

void main()
{
    fragColor = mix(texture(textureSampler1, texCoord), texture(textureSampler2, texCoord), 0.5);
    fragColor *= normalize(outColor + extColor);
}