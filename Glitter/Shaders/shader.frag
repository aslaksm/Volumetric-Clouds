#version 430 core

uniform layout (location = 4) vec2 resolution;
uniform layout (location = 5) float time;

layout(binding = 0) uniform sampler3D noise;

out vec4 color;

float to_range(float v, float l0, float h0, float ln, float hn) {
  return ln + ((v - l0) * (hn - ln))/(h0 - l0);

}

float get_sample(vec3 p){
  // return snoise(p*4.3);
  vec4 s = texture(noise, p+vec3(0,0,0));
  return s.x;
  return to_range(s.x, (s.y * 0.125 + s.z * 0.15) - 0.0, 1, 0, 1);
  // return to_range(s.x, (s.y * 0.625 + s.z * 0.25) - 1.0, 1, 0, 1);
}

float lightmarch(vec3 point, vec3 sun) {
  // return vec3(1.);
  float dist = 0.0;
  float steps = 4;
  float density = 0.0;
  for (int i = 0; i < steps ; i++) {
    vec3 p = point + sun*dist;
    density += get_sample(p);
    dist += 0.02;
  }
  return exp(-density);
}

// XXX: Why does sun impact cloud look so much?
// vec3 sun = vec3(0.0, 20.0, 0.0);
vec3 sun = vec3(1.6, 1.6, 0.1);

vec3 bg_color = vec3(0.8, 0.9725, 0.9725)*(400.+gl_FragCoord.y)/resolution.y/2.;

vec3 trace(vec3 origin, vec3 direction) {
  float STEP_SIZE = 0.01;
  int STEPS = 6;
  float dist = distance(vec3(0.0, 10.0, 0.0), origin);
  float transmittance = 1.0;
  float tot_density = 0.0;
  vec3 lightEnergy = vec3(0.0);

  vec3 p = origin + direction*dist;
  // if (p.y > 0.5)
  // {
    for (int i = 0; i < STEPS ; i++) {
      p = origin + direction*dist;
      float density = get_sample(p);
      if (density > 0.0) {
        // Cloud detected: start making smaller steps
        // STEP_SIZE = 0.05;
        float lightTransmittance = lightmarch(p, sun);

        lightEnergy += density * transmittance * lightTransmittance * 256*STEP_SIZE/STEPS;
        transmittance *= exp(-density * STEP_SIZE);

        if (transmittance < 0.1) break;
      }
    }
  // }
  dist += STEP_SIZE;
  // return vec3(transmittance);
  return bg_color * transmittance + lightEnergy * vec3(1.0, 1.0, 1.0);
}

void main() {
  vec3 bg_color = vec3(0.8, 0.9725, 0.9725)*(400.+gl_FragCoord.y)/resolution.y/2.;

  vec2 coord = (2*gl_FragCoord.xy - resolution)/resolution.x;

  vec3 direction = vec3(0.0, 0.80, 1.0);
  direction = normalize(vec3(direction.xy + coord, direction.z))/10;

  vec3 origin = vec3(0.0+time, 0.0, -5.0);

  vec3 transmittance = trace(origin, direction);

  vec3 new_color = vec3(transmittance);
  // vec3 new_color = vec3(transmittance)+bg_color;

  color = vec4(new_color, 1.0);

  // color = vec4(vec3(get_sample(vec3(gl_FragCoord.xy/resolution.x,time))), 1);
  // color = texture(noise, vec3(gl_FragCoord.xy/resolution.x,time));
 }
