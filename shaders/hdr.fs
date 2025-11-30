#version 460 core

uniform float exposure;
uniform float gamma;
uniform bool useHdr;
uniform sampler2D hdrColorTexture;
uniform float viewportXScale;
uniform float viewportYScale;

uniform int tonemappingAlgo;

in vec2 fragTexCoords;

out vec4 FragColor;

const mat3 ACESInputMat = mat3(
    0.59719, 0.35458, 0.04823,
    0.07600, 0.90834, 0.01566,
    0.02840, 0.13383, 0.83777
);

const mat3 ACESOutputMat = mat3(
     1.60475, -0.53108, -0.07367,
    -0.10208,  1.10813, -0.00605,
    -0.00327, -0.07276,  1.07602
);

vec3 RRTAndODTFit(vec3 v)
{
    vec3 a = v * (v + 0.0245786) - 0.000090537;
    vec3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
    return a / b;
}

void main()
{
    vec3 hdrColor = texture(hdrColorTexture, vec2(fragTexCoords.x * viewportXScale, fragTexCoords.y * viewportYScale)).rgb;
    // reinhard
    // vec3 result = hdrColor / (hdrColor + vec3(1.0));
    vec3 result = vec3(0.0);
    if(useHdr)
    {
        if(tonemappingAlgo == 0)
        {
            //reinhard
            result = vec3(1.0) - exp(-hdrColor * exposure);
        }
        else if(tonemappingAlgo == 1)
        {
            //uncharted
            float A = 0.15;
            float B = 0.50;
            float C = 0.10;
            float D = 0.20;
            float E = 0.02;
            float F = 0.30;
            float W = 11.2;
            vec3 color = hdrColor * exposure;
            color = ((color * (A * color + C * B) + D * E) / (color * (A * color + B) + D * F)) - E / F;
            float white = ((W * (A * W + C * B) + D * E) / (W * (A * W + B) + D * F)) - E / F;
            color /= white;
            result = color;
        }else if(tonemappingAlgo == 2)
        {
            //filmic
            vec3 color = max(vec3(0.), hdrColor - vec3(0.004));
	        color = (color * (6.2 * color + 0.5)) / (color * (6.2 * color + 1.7) + 0.06);
	        result = color;
        }else if(tonemappingAlgo == 3)
        {   
            //ACES
            vec3 color = hdrColor * ACESInputMat;

            // Apply RRT and ODT
            color = RRTAndODTFit(color);

            color = color * ACESOutputMat;
            color = clamp(color, 0.0, 1.0);
            result = color;
        }else if(tonemappingAlgo == 4)
        {
            //ACES filmic
            const float a = 2.51;
            const float b = 0.03;
            const float c = 2.43;
            const float d = 0.59;
            const float e = 0.14;
            result = clamp((hdrColor * (a * hdrColor + b)) / (hdrColor * (c * hdrColor + d ) + e), 0.0, 1.0);
        }
    }else
    {
        result = hdrColor;
    }
    result = pow(result, vec3(1.0 / gamma));
    FragColor = vec4(result, 1.0);
}