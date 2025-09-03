#version 400 core

uniform mat4 viewMat;
uniform mat4 clipMat;
uniform int axisLength;

flat out int vertexID;

void main()
{
    if(gl_VertexID % 2 == 0){
        gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
    }else{
        int directionMask = 0x01 << (gl_VertexID / 2);
        gl_Position = vec4(directionMask & 0x01, directionMask & 0x02, directionMask & 0x04, 1.0);
    }

    //axes are scaled after projection so as not to be dwarfed after perspective division
    gl_Position = mat4(axisLength, 0, 0, 0, 
                       0, axisLength, 0, 0,
                       0, 0, axisLength, 0,
                       0, 0, 0, 1) * clipMat * viewMat * gl_Position;

    vertexID = gl_VertexID;
}
