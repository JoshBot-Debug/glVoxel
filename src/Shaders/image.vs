#version 330 core

layout (location = 0) in vec3 in_Position;

out vec3 f_TexCoords;

uniform mat4 u_View;
uniform mat4 u_Projection;

void main()
{
  f_TexCoords = in_Position;
  vec4 pos = u_Projection*u_View*vec4(in_Position, 1.0);
  gl_Position = pos.xyww;
}  