#version 460 core

// ins
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 tangent;

layout(location = 4) in mat4 model; //instanced

//uniforms
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0); 
}