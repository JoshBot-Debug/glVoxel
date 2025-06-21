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

in vec3 f_Normal;
in vec3 f_Position;
in vec4 f_Color;
flat in int f_MaterialIndex;

uniform vec3 u_CameraPosition;
uniform Material u_Material;
uniform Light u_Light;

uniform sampler2D colorPalette;

void main()
{
  vec3 diffuseColor=f_Color.rgb;
  
  // Ambient
  vec3 ambient=u_Light.ambient*diffuseColor;
  
  // Diffuse
  vec3 lightDirection=normalize(u_Light.position-f_Position);
  vec3 diffuse=u_Light.diffuse*(max(dot(f_Normal,lightDirection),0.)*diffuseColor);
  
  // Specular
  vec3 viewDirection=normalize(u_CameraPosition-f_Position);
  vec3 reflectDirection=reflect(-lightDirection,f_Normal);
  float spec=pow(max(dot(viewDirection,reflectDirection),0.),u_Material.shininess);
  vec3 specular=u_Light.specular*(spec*u_Material.specular);
  
  FragColor=vec4(ambient+diffuse+specular,1.);
}