#version 330 core

out vec4 FragColor;

struct Material{
  vec3 diffuse;
  vec3 specular;
  float shininess;
};

struct Light{
  vec3 position;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

in vec3 f_Color;
in vec3 f_Normal;
in vec3 f_Position;
in vec2 f_TexCoord;

uniform vec3 u_CameraPosition;
uniform Material u_Material;
uniform Light u_Light;

void main()
{
  vec3 nNormal=normalize(f_Normal);
  
  // Ambient
  vec3 ambient=u_Light.ambient*u_Material.diffuse;
  
  // Diffuse
  vec3 lightDirection=normalize(u_Light.position-f_Position);
  float diff=max(dot(nNormal,lightDirection),0.);
  vec3 diffuse=u_Light.diffuse*(diff*u_Material.diffuse);
  
  // Specular
  vec3 viewDirection=normalize(u_CameraPosition-f_Position);
  vec3 reflectDirection=reflect(-lightDirection,nNormal);
  float spec=pow(max(dot(viewDirection,reflectDirection),0.),u_Material.shininess);
  vec3 specular=u_Light.specular*(spec*u_Material.specular);
  
  // FragColor=vec4(ambient+diffuse+specular+f_Color,1.);
  FragColor=vec4(f_Color,1.);
}