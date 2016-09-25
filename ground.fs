// SnowRacer
// Kristoffer 2009-04

// ground.fs
// ljus 0 - sol/måne
//    1,2 - strålkastare (riktade)
//      3 - kupebelysning (rundstrålande från bilen)

varying vec3 normal;
varying vec3 position;
const vec4 light_off = vec4(0.0, 0.0, 0.0, 0.0);


void main()
{
  vec3 nnorm = normalize(normal);
  
  //sol/måne
  vec3 lightnormal = normalize(gl_LightSource[0].position.xyz/gl_LightSource[0].position.w - position);
  gl_FragColor = gl_LightSource[0].diffuse * gl_FrontMaterial.diffuse * clamp(dot(nnorm, lightnormal), 0.0, 1.0);

  // strålkastare
  float dist = length(gl_LightSource[1].position.xyz/gl_LightSource[1].position.w - position);
  float atten = 1.0 / (gl_LightSource[1].constantAttenuation +
					             gl_LightSource[1].linearAttenuation * dist +
					             gl_LightSource[1].quadraticAttenuation * dist * dist);

  lightnormal = normalize(gl_LightSource[1].position.xyz/gl_LightSource[1].position.w - position);
  gl_FragColor += gl_LightSource[1].diffuse * gl_FrontMaterial.diffuse * clamp(dot(nnorm, lightnormal), 0.0, 1.0) * atten *
                pow(clamp(-dot(normalize(gl_LightSource[1].spotDirection), lightnormal), 0.0, 1.0),gl_LightSource[1].spotExponent);

  // (ungefär samma dämpning)
  lightnormal = normalize(gl_LightSource[2].position.xyz/gl_LightSource[2].position.w - position);
  gl_FragColor += gl_LightSource[2].diffuse * gl_FrontMaterial.diffuse * clamp(dot(nnorm, lightnormal), 0.0, 1.0) * atten *
                pow(clamp(-dot(normalize(gl_LightSource[2].spotDirection), lightnormal), 0.0, 1.0),gl_LightSource[2].spotExponent);
      
  
  //kupe
  lightnormal = normalize(gl_LightSource[3].position.xyz/gl_LightSource[3].position.w - position);
  dist = length(gl_LightSource[3].position.xyz/gl_LightSource[3].position.w - position);
  atten = 1.0 / (gl_LightSource[3].constantAttenuation +
					       gl_LightSource[3].linearAttenuation * dist +
					       gl_LightSource[3].quadraticAttenuation * dist * dist);
  gl_FragColor += gl_LightSource[3].diffuse * gl_FrontMaterial.diffuse * clamp(dot(nnorm, lightnormal), 0.0, 1.0) * atten;
					       
					       
}
