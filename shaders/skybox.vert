#version 330 core
layout (location = 0) in vec3 position;
out vec3 TexCoords;

uniform mat4 project;
uniform mat4 view;

// mineswep skybox vertex shader

void main()
{
// Optimized for early depth testing skybox; drawn last
    TexCoords = position;
    vec4 pos = project * view * vec4(position, 1.0);
    gl_Position = pos.xyww;
    /*gl_Position =   project * view * vec4(position, 1.0);  
    TexCoords = position;*/
} 