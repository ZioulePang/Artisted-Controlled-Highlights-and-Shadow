#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 normal;
in vec3 fragPos;

uniform sampler2D myTexture;
uniform sampler2D characterTexture;
uniform vec3 lightPos;

uniform float anis;
uniform float sharp;

float smoothClamp(float edge0,float edge1,float x){
    if(x < edge0 ) return 0.3;
    if(x > edge1 ) return 1;
    float t = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
    return t * t * (3.0f - 2.0f * t);
}

float anisotropyAndSharpness(vec2 texcoords)
{
    vec2 center = vec2(0.5, 0.5);
    float sigma = 0.1;
    vec2 diff = texcoords - center;
    float Intensity = exp(-dot(diff, diff) / (2.0 * sigma * sigma));
    float alpha = 1.0f - anis;

    float f = -1.0f * alpha * pow(texcoords.x,2) - (1/alpha) * pow(abs(texcoords.y),2.0f - sharp);

    return Intensity;
}


void main()
{   
   vec3 lightDir = normalize(fragPos - lightPos);
 
   vec2 light_texcoords = lightDir.xy * 0.5f + 0.5f;

   float intensity = anisotropyAndSharpness(light_texcoords);

   vec3 result = texture(myTexture,light_texcoords).rgb * intensity;

   FragColor = vec4(result,1.0f);
}