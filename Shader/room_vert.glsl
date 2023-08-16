#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

out vec2 TexCoords;
out vec3 normal;
out vec3 fragPos;
out vec3 aTangent;
out vec3 abitangent;
out vec4 fragPos_lightSpace;
out mat3 TBN;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

void main()
{
    TexCoords = aTexCoords;    
    normal = aNormal;
    fragPos = vec3(model * vec4(aPos,1.0f));
    aTangent = normalize(tangent);
    abitangent = normalize(bitangent);

    vec3 T = normalize(vec3(model * vec4(aTangent,0.0f)));
    vec3 B = normalize(vec3(model * vec4(abitangent,0.0f)));
    vec3 N = normalize(vec3(model * vec4(normal,0.0f)));

    TBN = transpose(mat3(T,B,N));

    fragPos_lightSpace = lightSpaceMatrix * vec4(fragPos,1.0);

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}