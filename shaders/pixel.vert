#version 330 core
layout (location = 0) in vec3 aPos;

/*uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;*/

//uniform mat4 pos;
uniform mat4 scale;
uniform mat4 project;
uniform vec2 pos;

void main()
{
    //gl_Position = pos * scale * vec4(aPos, 1.0f);
	vec4 vPos = vec4(aPos.x + pos.x, aPos.y + pos.y, aPos.z, 1.0f);
    gl_Position = project * scale * vPos;
}