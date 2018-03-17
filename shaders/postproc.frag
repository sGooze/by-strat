#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform float fade;
uniform sampler2D screenTexture;

void main()
{ 
    FragColor = texture(screenTexture, TexCoords) * vec4(fade, fade, fade, 1.0f);
}