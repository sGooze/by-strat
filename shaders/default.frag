#version 330 core
out vec4 FragColor;

in vec2 texCoord;

uniform sampler2D PlaneTex;

void main()
{
   /*float x = gl_FragCoord.x/640.0;
   float y = gl_FragCoord.y/480.0;
   FragColor = vec4(x, y, 1.0f, 1.0f) * min(fade, 1.0f);*/
   //FragColor = texture(PlaneTex, texCoord) * fade;
   FragColor = texture(PlaneTex, texCoord);
}