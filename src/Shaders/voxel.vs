#version 330 core

out vec3 f_Color;
out vec3 f_Normal;
out vec3 f_Position;
out vec2 f_TexCoord;

layout(location=0)in vec3 in_Position;
layout(location=1)in vec3 in_Normal;
layout(location=2)in vec2 in_TexCoord;

layout(location=3)in uint in_iType;
layout(location=4)in mat4 in_iModel;

uniform mat4 u_View;
uniform mat4 u_Projection;

void main()
{
  if(in_iType == uint(1)) { f_Color=vec3(0.,0.6,0.); }

  f_Normal=in_Normal;
  f_Position=vec3(in_iModel*vec4(in_Position,1.));
  f_TexCoord=in_TexCoord;
  
  gl_Position=u_Projection*u_View*in_iModel*vec4(in_Position,1.);
}