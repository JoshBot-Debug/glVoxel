#version 330 core

out vec4 FragColor;

struct Light{
  vec3 position;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

uniform Light u_Light;
uniform vec3 u_Camera;
uniform sampler2D u_Texture;

in vec2 f_TexCoords;

void main()
{
  vec4 color=texture(u_Texture,f_TexCoords);
  // if(color.a == 0.)discard;
  FragColor=color;
}