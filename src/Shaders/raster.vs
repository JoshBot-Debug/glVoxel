#version 330 core

out vec3 f_Position;
out vec3 f_Normal;
out vec4 f_Color;
flat out int f_MaterialIndex;

layout(location=0)in vec3 in_Position;
layout(location=1)in vec3 in_Normal;
layout(location=2)in int in_Color;
layout(location=3)in int in_MaterialIndex;

uniform mat4 u_View;
uniform mat4 u_Projection;

vec4 decodeRGBA(int packedColor) {
  float a = float((packedColor >> 24) & 0xFF) / 255.0;
  float b = float((packedColor >> 16) & 0xFF) / 255.0;
  float g = float((packedColor >> 8) & 0xFF) / 255.0;
  float r = float(packedColor & 0xFF) / 255.0;
  return vec4(r, g, b, a);
}

void main()
{ 
  f_Normal=in_Normal;
  f_Position=in_Position;
  f_Color=decodeRGBA(in_Color);
  f_MaterialIndex=in_MaterialIndex;
  
  gl_Position=u_Projection*u_View*vec4(in_Position,1.);
}