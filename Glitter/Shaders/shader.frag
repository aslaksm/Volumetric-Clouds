#version 430 core

uniform layout (location = 4) vec2 resolution;
uniform layout (location = 5) float time;
uniform layout (location = 6) vec3 orig_in;
uniform layout (location = 7) float dist_in;
uniform layout (location = 8) vec2 look_in;
uniform layout (location = 9) vec2 steps_in;
uniform layout (location = 10) int num_steps_in;

layout(binding = 0) uniform sampler2D coverage;
layout(binding = 1) uniform sampler3D noise;

out vec4 color;
float MAX_OUTER = 600000;
float DETAIL = 400000;
// float MAX_OUTER = 1920;
float PLANET_SPHERE = 30000.0;
float CLOUDS_INNER_SPHERE = 350000.0;
float CLOUDS_OUTER_SPHERE = 385000.0;


// https://gist.github.com/wwwtyro/beecc31d65d1004f5a9d
float ray_sphere_intersect(vec3 r0, vec3 rd, vec3 s0, float sr) {
    // - r0: ray origin
    // - rd: normalized ray direction
    // - s0: sphere center
    // - sr: sphere radius
    // - Returns distance from r0 to first intersecion with sphere,
    //   or -1.0 if no intersection.
    float a = dot(rd, rd);
    vec3 s0_r0 = r0 - s0;
    float b = 2.0 * dot(rd, s0_r0);
    float c = dot(s0_r0, s0_r0) - (sr * sr);
    if (b*b - 4.0*a*c < 0.0) {
        return -1.0;
    }
    return (-b - sqrt((b*b) - 4.0*a*c))/(2.0*a);
}

float remap(float v, float l0, float h0, float ln, float hn) {
  return ln + ((v - l0) * (hn - ln))/(h0 - l0);
}

float get_coverage_sample(vec3 p){
  p = p + vec3(0, 0 ,time*1000);
  float x = remap(p.x, -MAX_OUTER/2., MAX_OUTER/2., 0.0, 1.0);
  float z = remap(p.z, 0.0, MAX_OUTER, 0.0, 1.0);

  vec4 c = texture(coverage, vec2(x, z));
  return c.r;
}

float get_sample(vec3 p){
  p = p + vec3(0, 0, time*10000);
  float x = remap(p.x, -DETAIL, DETAIL, 0.0, 1.0);
  float y = remap(p.y, -DETAIL, DETAIL, 0.0, 1.0);
  float z = remap(p.z, 0.0, DETAIL, 0.0, 1.0);

  vec4 s = texture(noise, vec3(x, y, z));
  // vec4 s = texture(noise, p);
  // float perlin = 0.5*(s.r + s.g);
  return s.r * get_coverage_sample(p);

  // return remap(perlin, -0.5 - s.a, 1.0, 0.0, 1.0);
  // return remap(perlin, -1.0 - s.a, 1.0, 0.0, 1.0) * get_coverage_sample(p);
}


// Courtesy of A. Schneider
float henyey_greenstein(vec3 light_vec, vec3 view_vec, float g) {
  float cos_angle = dot(normalize(light_vec), normalize(view_vec));
  return ((1.0 - g * g) / pow((1.0 + g * g - 2.0 * g * cos_angle), 3.0 / 2.0)) / 4.0 * 3.1415;
}

float beer(float density) {
  return exp(-density);
}

float lightmarch(vec3 point, vec3 sun) {
  // return 1.0;
  float dist = 0.0;
  float steps = 4;
  float density = 0.0;
  for (int i = 0; i < steps ; i++) {
    vec3 p = point + sun*dist;
    density += get_sample(p);
    dist += steps_in.y;
  }
  return beer(density);
}

// vec3 sun = vec3(0.0, 20.0, 0.0);
vec3 sun = vec3(0.0, 1000000.0, 0.0);

vec3 bg_color = vec3(0.8, 0.9725, 0.9725)*(400.+gl_FragCoord.y)/resolution.y/2.;

vec4 trace(vec3 origin, vec3 direction) {
  // float STEP_SIZE = steps_in.x;

  float planet_dist = ray_sphere_intersect(origin, direction, vec3(0), PLANET_SPHERE);

  float start_dist = ray_sphere_intersect(origin, direction, vec3(0), CLOUDS_INNER_SPHERE);
  // XXX: Debugging only. We should always be inside the sphere
  if (start_dist == -1.0) return vec4(0);

  float end_dist = ray_sphere_intersect(origin, direction, vec3(0), CLOUDS_OUTER_SPHERE);

  float STEP_SIZE = (end_dist - start_dist) / num_steps_in;

  float transmittance = 1.0;
  float inScattering = 1.0;
  float tot_density = 0.0;
  float lightEnergy = 0.0;

  float dist = start_dist;
  for (int i = 0; i < num_steps_in ; i++) {
    vec3 p = origin + direction*dist;

      float density = get_sample(p);
      if (density > 0.0) {
        tot_density += density;
        transmittance *= exp(-density);
        inScattering *= (1 - exp(-density));
        float forward_scattering = henyey_greenstein(p, p-sun, 0.3);
        float light_density = lightmarch(p, sun);
        // lightEnergy = lightEnergy * attenuation + lightTransmittance * forward_scattering;
        lightEnergy += (2.0 * beer(light_density) * (1 - beer(2*light_density)) * forward_scattering) / pow(num_steps_in, 1.1);

      }
    dist += STEP_SIZE;
    }
  return vec4(lightEnergy * vec3(1.0, 1.0, 1.0), 1.0 - exp(-tot_density*0.1));
}

void main() {
  vec3 bg_color = vec3(0.5, 0.6725, 1.0)*(1500.-gl_FragCoord.y)/resolution.y;

  vec2 coord = (2*gl_FragCoord.xy - resolution)/resolution.x;

  vec3 direction = vec3(look_in, 1.0);
  direction = normalize(vec3(direction.xy + coord, direction.z));

  vec3 origin = orig_in;

  if (direction.y > 0.0)
  color = trace(origin, direction);
  else color = vec4(0);


  // Visualize textures
  // color = vec4(vec3(get_coverage_sample(vec3(gl_FragCoord.x, 0, gl_FragCoord.y-time*100))), 1);
  // color = vec4(vec3(get_sample(vec3(gl_FragCoord.x, 0, gl_FragCoord.y-time*100))), 1);
 }
