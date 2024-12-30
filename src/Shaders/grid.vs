#version 330 core

out vec3 f_Position;

layout(location=0)in vec3 in_Position;

uniform mat4 u_View;
uniform mat4 u_Projection;
uniform vec2 u_MousePosition;
uniform vec3 u_CameraPosition;
uniform float u_GridSize;

void main()
{ 
  // Step 1: Compute the mouse's world position
  vec4 mouseClipSpace = vec4(u_MousePosition, 0.0, 1.0); // x, y from mouse, z=0 (on near plane)
  
  // Step 2: Unproject mouse position to world space
  vec4 mouseWorldPos = inverse(u_Projection) * mouseClipSpace; // undo projection
  mouseWorldPos = inverse(u_View) * mouseWorldPos; // undo view transformation

  // Step 3: Now mouseWorldPos is in world coordinates, project to 2D plane if needed
  mouseWorldPos /= mouseWorldPos.w; // Normalize to homogeneous coordinates

  // Step 4: Snap to the nearest grid
  vec3 snappedPosition = round(mouseWorldPos.xyz / u_GridSize) * u_GridSize;

  // Step 5: Apply snapping to the object (translate the object by snapping the mouse position)
  vec3 worldPosition = snappedPosition + in_Position;

  // Pass the world position to the fragment shader
  f_Position = worldPosition;

  // Step 6: Apply the final transformation (projection + view)
  gl_Position = u_Projection * u_View * vec4(worldPosition, 1.0);
}