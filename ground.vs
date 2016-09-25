// SnowRacer
// Kristoffer 2009-04

// ground.vs

varying vec3 normal;
varying vec3 position;

void main()
{
  gl_Position = gl_ModelViewMatrix * gl_Vertex;
  position = gl_Position.xyz/gl_Position.w;
  gl_Position = gl_ProjectionMatrix * gl_Position;
 
  normal = gl_NormalMatrix * gl_Normal;
}
