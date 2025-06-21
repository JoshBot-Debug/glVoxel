#version 330 core

out vec4 FragColor;

struct Light{
  vec3 position;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

uniform Light u_Light;
uniform sampler2D u_Texture;
uniform vec3 u_Camera;

void main()
{
  FragColor=vec4(1.,1.,1.,1.);
}