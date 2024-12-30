#version 330 core

in vec3 f_Position;
out vec4 FragColor;

uniform vec4 u_GridColor;

void main(){
  FragColor=u_GridColor;
}
