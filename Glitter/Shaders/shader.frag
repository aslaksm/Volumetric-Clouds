#version 430 core

uniform layout (location = 4) vec2 resolution;
uniform layout (location = 5) float time;

layout(binding = 0) uniform sampler3D noise;

out vec4 color;

float get_sample(vec3 p){
  // return snoise(p*4.3);
  return texture(noise, p).x;
}

float lightmarch(vec3 point, vec3 sun) {
  return 1.;
  float dist = 0.0;
  float steps = 5;
  float density = 0.0;
  for (int i = 0; i < steps ; i++) {
    vec3 p = point + sun*dist;
    density += get_sample(p);
    dist += 1./steps;
  }
  return exp(-density);
}

vec3 sun = vec3(1.6, 1.6, 0.1);
vec3 trace(vec3 origin, vec3 direction) {
  float STEP_SIZE = 0.05;
  float dist = 0.4;
  float transmittance = 1.0;
  vec3 lightEnergy = vec3(0.0);

  for (int i = 0; i < 10 ; i++) {
    vec3 p = origin + direction*dist;
    float density = get_sample(p);
    if (density >= 0.0) {
      float lightTransmittance = lightmarch(p, sun);
      // transmittance += density;


      lightEnergy += density * (1 * STEP_SIZE) * transmittance * lightTransmittance;
      transmittance *= exp(-density * STEP_SIZE);
    }
  }
  dist += STEP_SIZE;
  // return vec3(transmittance);
  return transmittance*lightEnergy;
}

void main() {
  vec3 bg_color = vec3(0.8, 0.9725, 0.9725)*(400.+gl_FragCoord.y)/resolution.y/2.;

  vec2 coord = (2*gl_FragCoord.xy - resolution)/resolution.x;

  vec3 direction = normalize(vec3(coord, 1.0));
  vec3 origin = vec3(0.5, 0.5, 5+time);

  vec3 transmittance = trace(origin, direction);

  // vec3 new_color = vec3(transmittance);
  vec3 new_color = vec3(transmittance)+bg_color;

  color = vec4(new_color, 1.0);

  // color = texture(noise, vec3(gl_FragCoord.xy/resolution.x,time));
}
