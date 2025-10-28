#version 460 core

uniform sampler2D planeTexture;
uniform float checkerUnitWidth;
uniform float checkerUnitHeight;

in vec3 fragmentPos;

out vec4 fragColor;

void main()
{
    float texXCoord =  mod(fragmentPos.x, checkerUnitWidth) / checkerUnitWidth;
    float texZCoord =  mod(fragmentPos.z, checkerUnitHeight) / checkerUnitHeight;

    fragColor = texture(planeTexture, vec2(texXCoord, texZCoord));
}
