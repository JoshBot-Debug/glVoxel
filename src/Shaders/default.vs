#version 330 core

out vec4 FragColor;

struct Material {
  sampler2D diffuse;
  sampler2D specular;
  float shininess;
};

struct Light {
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
  vec3 nNormal = normalize(f_Normal);

  // Ambient
  vec3 ambient = u_Light.ambient * vec3(texture(u_Material.diffuse, f_TexCoord));

  // Diffuse
  vec3 lightDirection = normalize(u_Light.position - f_Position);  
  float diff = max(dot(nNormal, lightDirection), 0.0);
  vec3 diffuse = u_Light.diffuse * (diff * vec3(texture(u_Material.diffuse, f_TexCoord)));

  // Specular
  vec3 viewDirection = normalize(u_CameraPosition - f_Position);
  vec3 reflectDirection = reflect(-lightDirection, nNormal);  
  float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), u_Material.shininess);
  vec3 specular = u_Light.specular * (spec * vec3(texture(u_Material.specular, f_TexCoord)));  

  FragColor = vec4(ambient + diffuse + specular, 1.0);
}