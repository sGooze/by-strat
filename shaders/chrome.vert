#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTex;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec2 tex_scale;

out vec2 texCoord;
out vec2 frameCoords;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0f);
	texCoord = aTex * tex_scale;
	frameCoords = aTex;
}