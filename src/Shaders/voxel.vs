#version 330 core

out vec3 f_Position;
out vec3 f_Normal;

layout(location=0)in vec3 in_Position;
layout(location=1)in int in_Normal;

uniform mat4 u_View;
uniform mat4 u_Projection;

const vec3 NORMALS[6] = vec3[](
  vec3(0.0, 1.0, 0.0),  // TOP
  vec3(0.0, -1.0, 0.0), // BOTTOM
  vec3(0.0, 0.0, -1.0), // FRONT
  vec3(0.0, 0.0, 1.0),  // BACK
  vec3(-1.0, 0.0, 0.0), // LEFT
  vec3(1.0, 0.0, 0.0)   // RIGHT
);

void main()
{ 
  f_Normal=NORMALS[int(in_Normal)];
  f_Position=in_Position;
  
  gl_Position=u_Projection*u_View*vec4(in_Position,1.);
}