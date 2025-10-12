#version 400 core

uniform float thickness;
uniform float axisLength;

uniform mat4 viewMat;
uniform mat4 projectionMat;

layout(points) in;
layout(triangle_strip, max_vertices = 18) out;

in VS_OUT
{
    int vertexID;
    mat4 modelMat;
}
gsIn[]; // one at input

out vec4 outColor;

// rescales the vertex so that is appears to be of the same size in ndc
vec4 rescaleVertex(vec3 inputVertex, vec3 linearDimensions)
{
    float scale = -(viewMat * gsIn[0].modelMat * vec4(inputVertex, 1.0)).z;
    return projectionMat * viewMat * gsIn[0].modelMat
           * vec4(linearDimensions * scale * inputVertex, 1.0);
}

vec2 vertices[4] = vec2[4](vec2(1.0, 0), vec2(0, -1.0), vec2(-1.0, 0), vec2(0, 1.0));

void main()
{
    int vertId = gsIn[0].vertexID;

    int directionMask = 0x01 << (vertId);
    outColor = vec4((directionMask & 0x01), (directionMask & 0x02) >> 1,
                    (directionMask & 0x04) >> 2, 1.0);

    // output the side faces
    for (int i = 0; i < 5; ++i)
    {
        if (vertId == 0)
        {
            gl_Position = rescaleVertex(vec3(vertices[i % 4].xy, 0),
                                        vec3(thickness, thickness, 1.0));
            EmitVertex();

            gl_Position = rescaleVertex(vec3(vertices[i % 4].xy, axisLength),
                                        vec3(thickness, thickness, axisLength));

            EmitVertex();
        }
        else if (vertId == 1)
        {
            gl_Position = rescaleVertex(vec3(vertices[i % 4].x, 0, vertices[i % 4].y),
                                        vec3(thickness, 1.0, thickness));
            EmitVertex();

            gl_Position = rescaleVertex(vec3(vertices[i % 4].x, axisLength, vertices[i % 4].y),
                                        vec3(thickness, axisLength, thickness));
            EmitVertex();
        }
        else if (vertId == 2)
        {
            gl_Position = rescaleVertex(vec3(0, vertices[i % 4].xy),
                                        vec3(1.0, thickness, thickness));
            EmitVertex();

            gl_Position = rescaleVertex(vec3(axisLength, vertices[i % 4].xy),
                                        vec3(axisLength, thickness, thickness));
            EmitVertex();
        }
    }
    EndPrimitive();

    for (int k = 0; k < 2; ++k) // output base triangles
    {
        for (int j = 0; j < 4; ++j) // emit vertices for bases
        {
            if (vertId == 0)
            {
                gl_Position = rescaleVertex(vec3(vertices[j].xy, axisLength * k),
                                            vec3(thickness, thickness, axisLength));

                EmitVertex();
            }
            else if (vertId == 1)
            {
                gl_Position = rescaleVertex(vec3(vertices[j].x, axisLength * k, vertices[j].y),
                                            vec3(thickness, axisLength, thickness));

                EmitVertex();
            }
            else if (vertId == 2)
            {
                gl_Position = rescaleVertex(vec3(axisLength * k, vertices[j].xy),
                                            vec3(axisLength, thickness, thickness));

                EmitVertex();
            }
        }
        EndPrimitive();
    }
}