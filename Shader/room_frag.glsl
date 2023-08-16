#version 330 core
layout (location = 2) out vec4 FragColor;

in vec2 TexCoords;
in vec3 normal;
in vec3 fragPos;
in vec3 aTangent;
in vec3 abitangent;
in vec3 light_normal;
in vec4 fragPos_lightSpace;
in mat3 TBN;


uniform sampler2D myTexture;
uniform sampler2D depthMap;
struct Light{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    vec3 lightPos;
};

uniform Light myLight;
uniform vec3 viewPos;
uniform float shiness;
uniform vec4 rim;

uniform float scale_x;
uniform float scale_y;

uniform float rotation_x;
uniform float rotation_y;
uniform float rotation_z;

uniform float translate_x;
uniform float translate_y;

uniform float split_x;
uniform float split_y;

uniform float square_num;
uniform float square_scale;

uniform float specular_scale;

uniform bool normalShading;
uniform bool toonShading;

uniform float near_plane;
uniform float far_plane;

float toonRange = 0.44f;
vec3 shadowColor = vec3(0.55,0.55,0.65) * texture(myTexture,TexCoords).rgb;

float DegreeToRadian = 0.0174533f;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}


float ShadowCalculation(vec4 fragPosLightSpace)
{

    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    projCoords = projCoords * 0.5 + 0.5;

    float closestDepth = texture(depthMap, projCoords.xy).r; 
    closestDepth = LinearizeDepth(closestDepth) / far_plane;
   

    float currentDepth = projCoords.z;
    float bias = 0.003f;

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(depthMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(depthMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0f;
    if(projCoords.z > 1.0)
        shadow = 0.0;
    return shadow;
}


float smoothClamp(float edge0,float edge1,float x){
    if(x < edge0 ) return 0.3;
    if(x > edge1 ) return 1;
    float t = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
    return t * t * (3.0f - 2.0f * t);
}


void main()
{   
   
    //ambient
    vec3 ambient = myLight.ambient * texture(myTexture,TexCoords).rgb;

    vec4 RimColor = rim * vec4(ambient,1.0f);
    //diffuse
    vec3 norm = vec3(0,0,1);
    vec3 lightDir =TBN * normalize(myLight.lightPos - fragPos);
    float diff = max(dot(norm,lightDir),0.0f);
    diff = diff * 0.5 + 0.5;
    vec3 diffuse = diff * myLight.diffuse * texture(myTexture,TexCoords).rgb;
    
    
    float halfLambert = dot(norm,lightDir) * 0.5 + 0.5;
    float ramp = smoothClamp(0,0.25f,halfLambert - toonRange);
    //vec3 toonDiff = halfLambert > 0.5f ? diffuse : shadowColor * ramp;
    vec3 toonDiff = shadowColor + (diffuse - shadowColor) * ramp;

    vec3 viewDir = TBN * normalize(viewPos - fragPos);

    //Stylised Highlight
    vec3 halfVector = normalize(viewDir + lightDir);

    //1.Scale
    halfVector = normalize(halfVector - scale_x * halfVector.x * vec3(1,0,0));
    halfVector = normalize(halfVector - scale_y * halfVector.y * vec3(0,1,0));

    //2.Rotation
    float x_rad = rotation_x * DegreeToRadian;
    mat3 x_Rotation;
    x_Rotation[0] = vec3(1,0,0);
    x_Rotation[1] = vec3(0,cos(x_rad),sin(x_rad));
    x_Rotation[2] = vec3(0,-sin(x_rad),cos(x_rad));

    float y_rad = rotation_y * DegreeToRadian;
    mat3 y_Rotation;
    y_Rotation[0] = vec3(cos(y_rad),0,-sin(y_rad));
    y_Rotation[1] = vec3(0,1,0);
    y_Rotation[2] = vec3(sin(y_rad),0,cos(y_rad));

    float z_rad = rotation_z * DegreeToRadian;
    mat3 z_Rotation;
    z_Rotation[0] = vec3(cos(z_rad),sin(z_rad),0);
    z_Rotation[1] = vec3(-sin(z_rad),cos(x_rad),0);
    z_Rotation[2] = vec3(0,0,1);

    halfVector = z_Rotation * y_Rotation * x_Rotation * halfVector;

    //3.Translation
    halfVector = halfVector + vec3(translate_x,translate_y,0);
    halfVector = normalize(halfVector);

    //4.Split
    float signX = 1;
    if(halfVector.x < 0) signX = -1;

    float signY = 1;
    if(halfVector.y < 0) signY = -1;

    halfVector = halfVector - split_x * signX * vec3(1,0,0) - split_y * signY * vec3(0,1,0);
    halfVector = normalize(halfVector);

    //5.Square
    float square_X = acos(halfVector.x);
    float square_Y = acos(halfVector.y);
    float square_normal_X = sin(pow(2 * square_X,square_num));
    float square_normal_Y = sin(pow(2 * square_Y,square_num));
    
    halfVector = halfVector - square_scale * (square_normal_X * halfVector.x * vec3(1,0,0) + square_normal_Y * halfVector.y * vec3(0,1,0));
    halfVector = normalize(halfVector);

    //Stylized specular
    vec3 reflectDir = reflect(-lightDir,norm);
    float spec = pow(max(dot(viewDir,reflectDir),0.0f),shiness);

    float stylized_spec = pow(max(dot(norm,halfVector),0.0f),shiness);
    float w = fwidth(stylized_spec) * 1.0f;
    float lerp_parameter = smoothClamp(-w,w,stylized_spec + specular_scale - 1.0f);

    vec3 specular = myLight.specular * spec * texture(myTexture,TexCoords).rgb;
    vec3 stylized_specular = texture(myTexture,TexCoords).rgb  * lerp_parameter * 0.4;

    float f = 1.0f - clamp(dot(viewDir,norm),0.0f,1.0f);
    float smooth_f = smoothClamp(0.15f,0.65f,1.0f);
    float rim = smoothClamp(0.0f,0.5f,1.0f);

    vec3 rimColor = rim * RimColor.rgb * RimColor.a;

    //Bloom
    float bloom = pow(f,3.0f) * 3.0f * toonDiff.r;
    vec3 blinn_phong = ambient + diffuse + specular;

    float shadow = ShadowCalculation(fragPos_lightSpace);


    vec3 result;
    if(normalShading)
    {
        result = blinn_phong;
    }
    if(toonShading)
    {   
        result = (rimColor + (1.0f - shadow))*(toonDiff + stylized_specular) * texture(myTexture,TexCoords).rgb * 0.8  + toonDiff * 0.8 + stylized_specular * 0.4;
    }
    FragColor = vec4(result,bloom);

}