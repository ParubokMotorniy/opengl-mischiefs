#version 400 core

uniform float thickness;

uniform float cameraNear;
uniform float cameraDistance;

uniform float axisLength;

uniform mat4 viewMat;
uniform mat4 clipMat;

layout (points) in;
layout (triangle_strip, max_vertices = 18) out;

in VS_OUT
{
int vertexID;
} gsIn[]; //one at input

out vec4 outColor;

void main()
{
    float thicknessScaling = thickness * cameraDistance / cameraNear;
    float lengthScaling = axisLength * cameraDistance / cameraNear;

    vec2 vertices[4] = vec2[4](vec2(thicknessScaling, 0),
                               vec2(0, -thicknessScaling),
                               vec2(-thicknessScaling, 0),
                               vec2(0, thicknessScaling));

    int vertId = gsIn[0].vertexID; 

    int directionMask = 0x01 << (vertId);
    outColor = vec4((directionMask & 0x01), (directionMask & 0x02) >> 1, (directionMask & 0x04) >> 2, 1.0);

    mat4 mvpMat = clipMat * viewMat;
    

    //output the side faces
    for(int i = 0; i < 5; ++i)
    {
        if(vertId == 0)
        {
            gl_Position = mvpMat * vec4(vertices[i % 4].xy, 0, 1.0);

            EmitVertex();
            gl_Position = mvpMat * vec4(vertices[i % 4].xy, lengthScaling, 1.0);
            
            EmitVertex();
        }else if(vertId == 1)
        {
            gl_Position = mvpMat * vec4(vertices[i % 4].x, 0, vertices[i % 4].y, 1.0);
            
            EmitVertex();
            gl_Position = mvpMat * vec4(vertices[i % 4].x, lengthScaling, vertices[i % 4].y, 1.0);
            
            EmitVertex();
        }else if(vertId == 2)
        {
            gl_Position = mvpMat * vec4(0, vertices[i % 4].xy, 1.0);
            
            EmitVertex();
            gl_Position = mvpMat * vec4(lengthScaling, vertices[i % 4].xy, 1.0);
            
            EmitVertex();
        }
    }
    EndPrimitive();

    for(int k = 0; k < 2; ++k) //output base triangles
    {   
        for(int j = 0; j < 4; ++j) //emit vertices for bases
        {
            if(vertId == 0)
            {
                gl_Position = mvpMat * vec4(vertices[j].xy, lengthScaling * k, 1.0);
                
                EmitVertex();
            }else if(vertId == 1)
            {
                gl_Position = mvpMat * vec4(vertices[j].x, lengthScaling * k, vertices[j].y, 1.0);
                
                EmitVertex();
            }else if(vertId == 2)
            {
                gl_Position = mvpMat * vec4(lengthScaling * k, vertices[j].xy, 1.0);
                
                EmitVertex();
            }
        }
        EndPrimitive();
    }

}