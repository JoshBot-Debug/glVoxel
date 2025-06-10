#version 330 core

out vec3 f_Position;
out vec3 f_Normal;
flat out int f_ColorIndex;
flat out int f_MaterialIndex;

layout(location=0)in vec3 in_Position;
layout(location=1)in vec3 in_Normal;
layout(location=2)in int in_ColorIndex;
layout(location=3)in int in_MaterialIndex;

uniform mat4 u_View;
uniform mat4 u_Projection;

void main()
{ 
  f_Normal=in_Normal;
  f_Position=in_Position;
  f_ColorIndex=in_ColorIndex;
  f_MaterialIndex=in_MaterialIndex;
  
  gl_Position=u_Projection*u_View*vec4(in_Position,1.);
}