#version 400 core

in vec4 outColor;
out vec4 fragColor;

// red - right, green - up, blue - forward

void main() { fragColor = outColor; }
