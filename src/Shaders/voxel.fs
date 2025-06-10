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
flat in int f_ColorIndex;
flat in int f_MaterialIndex;

uniform vec3 u_CameraPosition;
uniform Material u_Material;
uniform Light u_Light;

void main()
{
  vec3 voxelColor;
  
  if(f_ColorIndex==1){
    voxelColor=vec3(.1,.8,.1);// GRASS (green)
  }else if(f_ColorIndex==2){
    voxelColor=vec3(.55,.27,.07);// DIRT (brown)
  }else if(f_ColorIndex==3){
    voxelColor=vec3(.5,.5,.5);// STONE (gray)
  }else if(f_ColorIndex==4){
    voxelColor=vec3(1.,1.,1.);// SNOW (white)
  }else{
    voxelColor=vec3(1.,0.,1.);// Fallback magenta for debugging
  }
  
  vec3 diffuseColor=voxelColor;
  
  // Ambient
  // vec3 ambient=u_Light.ambient*u_Material.diffuse;
  vec3 ambient=u_Light.ambient*diffuseColor;
  
  // Diffuse
  vec3 lightDirection=normalize(u_Light.position-f_Position);
  // vec3 diffuse=u_Light.diffuse*(max(dot(f_Normal,lightDirection),0.)*u_Material.diffuse);
  vec3 diffuse=u_Light.diffuse*(max(dot(f_Normal,lightDirection),0.)*diffuseColor);
  
  // Specular
  vec3 viewDirection=normalize(u_CameraPosition-f_Position);
  vec3 reflectDirection=reflect(-lightDirection,f_Normal);
  float spec=pow(max(dot(viewDirection,reflectDirection),0.),u_Material.shininess);
  vec3 specular=u_Light.specular*(spec*u_Material.specular);
  
  FragColor=vec4(ambient+diffuse+specular,1.);
  // FragColor=vec4(1.,1.,1.,1.);
}