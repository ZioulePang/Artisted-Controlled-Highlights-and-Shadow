#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

const float offset = 1.0 / 1000.0;  

vec2 offsets[9] = vec2[](
        vec2(-offset,  offset), 
        vec2( 0.0f,    offset), 
        vec2( offset,  offset), 
        vec2(-offset,  0.0f),  
        vec2( 0.0f,    0.0f),   
        vec2( offset,  0.0f),   
        vec2(-offset, -offset), 
        vec2( 0.0f,   -offset),
        vec2( offset, -offset) 
    );


float luminance(vec4 color){
	float luminance = dot(color.xyz, vec3(0.2126, 0.7152, 0.0722));
    return luminance;
}


float calEdge(){
    float Gx[9] = float[](-1,  0,  1,
					-2,  0,  2,
					-1,  0,  1);

	float Gy[9] = float[](-1, -2, -1,
						0,  0,  0,
						1,  2,  1);

	float texColor , edgeX , edgeY;
	for (int it = 0; it < 9; it++) {
		texColor = luminance(texture(screenTexture, TexCoords.st + offsets[it]));
		edgeX += texColor * Gx[it];
		edgeY += texColor * Gy[it];
	}

    float edge = 1 - abs(edgeX) - abs(edgeY);
    return edge;
}


void main()
{
    float edge = calEdge();
    vec4 edgeColor = vec4(0,0,0,1);
    vec4 backgroundColor = vec4(0.1f, 0.1f, 0.1f, 1.0f);
    vec4 withEdgeColor = mix(edgeColor, texture(screenTexture, TexCoords), edge);
    vec4 onlyEdgeColor = mix(edgeColor, backgroundColor, edge);
    vec4 result = mix(withEdgeColor, onlyEdgeColor,0.8f);
    FragColor = result + texture(screenTexture, TexCoords);

}

 