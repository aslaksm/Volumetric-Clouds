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
float SCALE_COVERAGE = 1000000;
float SCALE_DETAIL = 10000;
// float SCALE_COVERAGE = 1920;
float PLANET_SPHERE = 6300e3;
float CLOUDS_INNER_SPHERE = 3000.0;
float CLOUDS_OUTER_SPHERE = 3800.0;

float hash( float n )
{
    return fract(sin(n)*43758.5453);
}


// https://www.shadertoy.com/view/4dSBDt
float ray_sphere_intersect(vec3 origin, vec3 dir, vec3 spherePos, float sphereRad)
{
	vec3 oc = origin - spherePos;
	float b = 2.0 * dot(dir, oc);
	float c = dot(oc, oc) - sphereRad*sphereRad;
	float disc = b * b - 4.0 * c;
	if (disc < 0.0)
		return -1.0;    
    float q = (-b + ((b < 0.0) ? -sqrt(disc) : sqrt(disc))) / 2.0;
	float t0 = q;
	float t1 = c / q;
	if (t0 > t1) {
		float temp = t0;
		t0 = t1;
		t1 = temp;
	}
	if (t1 < 0.0)
		return -1.0;
    
    return (t0 < 0.0) ? t1 : t0;
}

float remap(float v, float l0, float h0, float ln, float hn) {
  return ln + (v - l0) * (hn - ln) / (h0 - l0);
}

float get_sample(vec3 p){
  p = p + vec3(0, 0, time*1000);
  float x = remap(p.x, -SCALE_DETAIL, SCALE_DETAIL, 0.0, 1.0);
  float y = remap(p.y, -SCALE_DETAIL, SCALE_DETAIL, 0.0, 1.0);
  float z = remap(p.z, 0.0, SCALE_DETAIL, 0.0, 1.0);
  vec4 s = texture(noise, vec3(x, y, z));

  return s.r;
}

float get_coverage(vec3 p) {
  float x = remap(p.x, -SCALE_COVERAGE/2., SCALE_COVERAGE/2., 0.0, 1.0);
  float z = remap(p.z, 0.0, SCALE_COVERAGE, 0.0, 1.0);

  vec4 c = texture(coverage, vec2(x, z));
  return c.r;
}

float sample_clouds(vec3 p){
  // return get_sample(p);
  p = p + vec3(0, 0 ,time*1000);
  float cov = get_coverage(p);
  return cov*get_sample(p);
}



// Courtesy of A. Schneider
float henyey_greenstein(vec3 light_vec, vec3 view_vec, float g) {
  float cos_angle = dot(normalize(light_vec), normalize(view_vec));
  return ((1.0 - g * g) / pow((1.0 + g * g - 2.0 * g * cos_angle), 1.5)) / 4.0 * 3.1415;
}

float beer(float density) {
  return exp(-density);
}

// FIXME
float lightmarch(vec3 point, vec3 sun_dir) {
  return 1.0;
  float steps = 4.;

  float ATM_END = PLANET_SPHERE + CLOUDS_OUTER_SPHERE;

  // If we're near the end of the box we take smaller steps, therefore T is scaled by step size
  float dist = ray_sphere_intersect(point, sun_dir, vec3(0, -PLANET_SPHERE, 0), ATM_END);
  float step_size = dist/steps;

  float density = 0.0;
  float T = 1.0;
  vec3 p = point + sun_dir*step_size;

  for (int i = 0; i < steps ; i++) {
    density = sample_clouds(p);
    T *= beer(density*step_size*steps_in.y*0.001);
    
    p += step_size*sun_dir;
  }
  return T;
}

// vec3 sun = vec3(0.0, 20.0, 0.0);
// Direction to the sun
vec3 sun_dir = normalize(vec3(0.6,-0.45,-0.8));

vec4 trace(vec3 origin, vec3 direction) {
  // float STEP_SIZE = steps_in.x;

  float ATM_START = PLANET_SPHERE + CLOUDS_INNER_SPHERE;
  float ATM_END = PLANET_SPHERE + CLOUDS_OUTER_SPHERE;


  float start_dist = ray_sphere_intersect(origin, direction, vec3(0, -PLANET_SPHERE, 0), ATM_START);
  // XXX: Debugging only. We should always be inside the sphere
  if (start_dist == -1.0) return vec4(0);

  float end_dist = ray_sphere_intersect(origin, direction, vec3(0, -PLANET_SPHERE, 0), ATM_END);
 
  // FIXME: More steps depending on distance. Fixed step size?
  float STEP_SIZE = (end_dist - start_dist) / num_steps_in;
  // float STEP_SIZE = 50;

  float T = 1.0;

  float inScattering = 1.0;
  float tot_density = 0.0;
  vec3 lightEnergy = vec3(0.0);
  vec3 color = vec3(0);
  vec3 bg = vec3(vec3(0.5, 0.6725, 1.0)*(1500.-gl_FragCoord.y)/resolution.y);

  float dist = start_dist;
  vec3 p = origin + direction*dist;

  p += direction*STEP_SIZE*hash(dot(direction, vec3(12.256, 2.646, 6.356)));

  for (int i = 0; i < num_steps_in ; i++) {
    p += direction*(STEP_SIZE+hash(dot(direction, vec3(12.256, 2.646, 6.356)))) + time;

      float density = sample_clouds(p);
      if (density > 0.0) {

        float lightT = lightmarch(p, sun_dir);
        float dT = exp(-density * STEP_SIZE * steps_in.y);
        T *= dT;

        float forward_scattering = henyey_greenstein(p, p-sun_dir, 0.3);
        
        color += density * STEP_SIZE * T * lightT * forward_scattering * inScattering / (steps_in.x*10);

        if (T < 0.01) break;
        // radiance *= density;
        // lightEnergy += steps_in.y * transmittance * (radiance) / density;
        // transmittance *= beer(density * STEP_SIZE);
        // lightEnergy = lightEnergy * transmittance + lightTransmittance * forward_scattering;
        // lightEnergy += (2.0 * beer(lightT) * (1 - beer(2*lightT)) * forward_scattering);
        // lightEnergy += light_density;
      }
    }
  color += 0.3;
  return vec4(color, 1.0 - T);
  // return vec4(lightEnergy * vec3(1.0, 1.0, 1.0), 1.0);
}

vec3 getdir(vec2 uv, vec3 orig, vec3 look_at, float zoom) {
    vec3 f = normalize(look_at - orig);
    vec3 r = cross(vec3(0.0,1.0,0.0),f);
    vec3 u = cross(f,r);
    vec3 c=orig+f*zoom;
    vec3 i=c+uv.x*r+uv.y*u;
    vec3 dir=i-orig;
    return normalize(dir);
}

void main() {

  vec2 uv=gl_FragCoord.xy/resolution.xy;
  uv-=vec2(0.5);//offset, so center of screen is origin
  uv.x*=resolution.x/resolution.y;//scale, so there is no rectangular distortion
   
  vec3 orig=vec3(0.0,0.0,0.0);
  vec3 look_at=vec3(0.0, 0.0, 1.0);
  float zoom=1.0;

  vec3 direction = getdir(uv, orig, look_at, zoom); 
    

  // vec2 coord = (2*gl_FragCoord.xy - resolution)/resolution.x;

  // vec3 direction = vec3(look_in, 1.0);
  // direction = normalize(vec3(direction.xy + coord, direction.z));

  vec3 origin = orig_in;

  if (direction.y > 0.0)
  color = trace(origin, direction);
  else color = vec4(0);


  // Visualize textures
  // color = vec4(vec3(sample_clouds(vec3(gl_FragCoord.x, 0, gl_FragCoord.y-time*100))), 1);
  // color = vec4(vec3(get_sample(vec3(gl_FragCoord.x, gl_FragCoord.y, time*100))), 1);
 }
