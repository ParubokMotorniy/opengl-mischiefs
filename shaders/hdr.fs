#version 460 core

uniform float exposure;
uniform sampler2D hdrColorTexture;
uniform float viewportXScale;
uniform float viewportYScale;

in vec2 fragTexCoords;

out vec4 FragColor;

//TODO: may want to make hdr optional

void main()
{
    const float gamma = 2.2;
    vec3 hdrColor = texture(hdrColorTexture, vec2(fragTexCoords.x * viewportXScale, fragTexCoords.y * viewportYScale)).rgb;
// vec3 hdrColor = texture(hdrColorTexture, fragTexCoords).rgb;
    // reinhard
    // vec3 result = hdrColor / (hdrColor + vec3(1.0));
    // exposure
    vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
    // also gamma correct while we're at it       
    result = pow(result, vec3(1.0 / gamma));
    FragColor = vec4(result, 1.0);
}