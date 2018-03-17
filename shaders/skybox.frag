#version 330 core
in vec3 TexCoords;
out vec4 color;

uniform samplerCube skybox;

void main()
{    
	vec3 fix_coords = vec3(1.0 - TexCoords.x, 1.0 - TexCoords.y, 1.0 - TexCoords.z);
	//color = vec4(fix_coords, 1.0f);
    color = texture(skybox, TexCoords);
}