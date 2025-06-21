#version 330 core

layout(location=0)in vec3 in_Position;

out vec2 f_TexCoords;

uniform mat4 u_View;
uniform mat4 u_Projection;

void main()
{
  const vec2 pos[6]=vec2[](
    vec2(-1.,-1.),// bottom-left
    vec2(1.,-1.),// bottom-right
    vec2(1.,1.),// top-right
    
    vec2(1.,1.),// top-right
    vec2(-1.,1.),// top-left
    vec2(-1.,-1.)// bottom-left
  );
  
  const vec2 uv[6]=vec2[](
    vec2(0.,0.),
    vec2(1.,0.),
    vec2(1.,1.),
    
    vec2(1.,1.),
    vec2(0.,1.),
    vec2(0.,0.)
  );
  
  // gl_Position = u_Projection*u_View*localPos;
  gl_Position=vec4(pos[gl_VertexID],0.,1.);
  f_TexCoords=uv[gl_VertexID];
}