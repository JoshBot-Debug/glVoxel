#version 330 core

layout(location=0)in vec3 in_Position;

out vec2 f_TexCoords;

uniform mat4 u_View;
uniform mat4 u_Projection;

void main()
{
  const vec2 pos[6] = vec2[](
    vec2(-1.0, -1.0), // bottom-left
    vec2( 1.0, -1.0), // bottom-right
    vec2( 1.0,  1.0), // top-right

    vec2( 1.0,  1.0), // top-right
    vec2(-1.0,  1.0), // top-left
    vec2(-1.0, -1.0)  // bottom-left
  );

  const vec2 uv[6] = vec2[](
    vec2(0.0, 0.0),
    vec2(1.0, 0.0),
    vec2(1.0, 1.0),

    vec2(1.0, 1.0),
    vec2(0.0, 1.0),
    vec2(0.0, 0.0)
  );
  
  // gl_Position = u_Projection*u_View*localPos;
  vec4 localPos=vec4(pos[gl_VertexID],0.,1.);
  gl_Position=localPos;
  f_TexCoords=uv[gl_VertexID];
}