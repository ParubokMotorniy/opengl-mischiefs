#version 460 core

uniform float exposure;
uniform float gamma;
uniform bool useHdr;
uniform sampler2D hdrColorTexture;
uniform float viewportXScale;
uniform float viewportYScale;

in vec2 fragTexCoords;

out vec4 FragColor;

void main()
{
    vec3 hdrColor = texture(hdrColorTexture, vec2(fragTexCoords.x * viewportXScale, fragTexCoords.y * viewportYScale)).rgb;
    // reinhard
    // vec3 result = hdrColor / (hdrColor + vec3(1.0));
    vec3 result = vec3(0.0);
    if(useHdr)
    {
        result = vec3(1.0) - exp(-hdrColor * exposure);
    }else
    {
        result = hdrColor;
    }
    result = pow(result, vec3(1.0 / gamma));
    FragColor = vec4(result, 1.0);
}