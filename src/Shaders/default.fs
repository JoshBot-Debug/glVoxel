#version 330 core

out vec3 f_Color;
out vec3 f_Normal;
out vec3 f_Position;
out vec2 f_TexCoord;

layout(location=0)in vec3 in_Position;
layout(location=1)in vec3 in_Normal;
layout(location=2)in vec2 in_TexCoord;
layout(location=3)in vec3 in_Tangent;
layout(location=4)in vec3 in_BiTangent;
layout(location=5)in vec3 in_Color;

layout(location=6)in mat4 in_iModel;
layout(location=10)in mat3 in_iNormalMatrix;
layout(location=13)in vec3 in_iColor;

uniform mat4 u_View;
uniform mat4 u_Projection;

void main(){ 
  f_Color=in_Color*in_iColor;
  f_Normal= in_iNormalMatrix * in_Normal;  
  f_Position=vec3(in_iModel*vec4(in_Position,1.));
  f_TexCoord=in_TexCoord;
  
  gl_Position=u_Projection*u_View*in_iModel*vec4(in_Position,1.);
}