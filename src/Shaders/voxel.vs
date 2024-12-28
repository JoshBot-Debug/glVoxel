#version 330 core

out vec3 f_Position;
out vec3 f_Normal;

layout(location=0)in vec3 in_Position;
layout(location=1)in vec3 in_Normal;

uniform mat4 u_View;
uniform mat4 u_Projection;

void main()
{ 
  f_Normal=in_Normal;
  f_Position=in_Position;
  
  gl_Position=u_Projection*u_View*vec4(in_Position,1.);
}