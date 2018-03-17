#version 330 core
out vec4 FragColor;

in vec2 texCoord;
in vec2 frameCoords;

uniform sampler2D PlaneTex;
uniform sampler2D FrameTex;

void main()
{
   vec4 tex = texture(PlaneTex, texCoord);
   vec4 frm = texture(FrameTex, frameCoords);
   FragColor = mix(tex, frm, frm.a); //texture(PlaneTex, texCoord);
}