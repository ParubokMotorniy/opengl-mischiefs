#version 460 core

uniform sampler2D basicTexture;

in vec2 texCoord;

out vec4 fragColor;

void main() { fragColor = texture(basicTexture, texCoord); }
