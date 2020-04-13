#version 430 core

uniform layout (location = 4) vec2 resolution;
uniform layout (location = 5) float time;
uniform layout (location = 6) vec3 orig_in;
uniform layout (location = 7) float height_in;
uniform layout (location = 8) bool cov_only_in;
uniform layout (location = 9) float trans;
uniform layout (location = 10) float scat;
uniform layout (location = 11) int num_steps_in;
uniform layout (location = 12) float light_step_size;

layout(binding = 0) uniform sampler2D coverage;
layout(binding = 1) uniform sampler3D noise;
layout(binding = 2) uniform sampler3D detail;

out vec4 color;
float SCALE_COVERAGE = 40000;
float SCALE_BASE = 5000;
float SCALE_DETAIL = 5000;

// float SCALE_COVERAGE = 4096;
// float SCALE_BASE = 512;
// float SCALE_DETAIL = 512;

float PLANET_SPHERE = 6300e3;
float CLOUDS_INNER_SPHERE = 3000.0;
float CLOUDS_OUTER_SPHERE = 3200.0 + height_in;
float RENDER_DIST = 40000;

#define BAYER_FACTOR 1.0/16.0
float bayerFilter[16u] = float[]
(
	0.0*BAYER_FACTOR, 8.0*BAYER_FACTOR, 2.0*BAYER_FACTOR, 10.0*BAYER_FACTOR,
	12.0*BAYER_FACTOR, 4.0*BAYER_FACTOR, 14.0*BAYER_FACTOR, 6.0*BAYER_FACTOR,
	3.0*BAYER_FACTOR, 11.0*BAYER_FACTOR, 1.0*BAYER_FACTOR, 9.0*BAYER_FACTOR,
	15.0*BAYER_FACTOR, 7.0*BAYER_FACTOR, 13.0*BAYER_FACTOR, 5.0*BAYER_FACTOR
);


float hash( float n )
{
    return fract(sin(n)*43758.5453);
}


// Taken from https://www.shadertoy.com/view/4dSBDt
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


float get_height_fraction(vec3 p) {
  return (p.y -  CLOUDS_INNER_SPHERE) / (CLOUDS_OUTER_SPHERE - CLOUDS_INNER_SPHERE);
}

// Cumulus clouds go from 0.0 to 0.5
float cumulus_function(float height) {
  return min(smoothstep(0.0, 0.3, height), smoothstep(0.5, 0.4, height));
}

// Stratus clouds go from 0.0 to 0.2
float stratus_function(float height) {
  return min(smoothstep(0.0, 0.1, height), smoothstep(0.2, 0.1, height));
}

//


float get_sample(vec3 p){
  float x = remap(p.x, -SCALE_BASE/2, SCALE_BASE/2, 0.0, 1.0);
  float y = remap(p.y, -SCALE_BASE/2, SCALE_BASE/2, 0.0, 1.0);
  float z = remap(p.z, -SCALE_BASE/2, SCALE_BASE/2, 0.0, 1.0);

  vec4 n = texture(noise, fract(vec3(x, y, z)));

  float fbm = n.g * 0.625 + n.b * .25 + n.a * 0.125;
  float base_cloud = remap(n.r, - (1.0 - fbm), 1.0, 0.0, 1.0);

  return base_cloud;
}

float get_detail_sample(vec3 p) {
  float x = remap(p.x, -SCALE_DETAIL/2, SCALE_DETAIL/2, 0.0, 1.0);
  float y = remap(p.y, -SCALE_DETAIL/2, SCALE_DETAIL/2, 0.0, 1.0);
  float z = remap(p.z, -SCALE_DETAIL/2, SCALE_DETAIL/2, 0.0, 1.0);

  vec4 d = texture(detail, fract(vec3(x,y,z)));

  float height_fraction = get_height_fraction(p);
  float hd_fbm = d.r * 0.625 + d.g * .25 + d.b * 0.125;
  float hd_mod = mix(hd_fbm, 1.0 - hd_fbm, clamp(height_fraction * 10.0, 0.0, 1.0));

  return hd_mod;
}

float get_coverage(vec3 p) {
  float x = remap(p.x, -SCALE_COVERAGE/2., SCALE_COVERAGE/2., 0.0, 1.0);
  float z = remap(p.z, -SCALE_COVERAGE/2, SCALE_COVERAGE/2, 0.0, 1.0);

  vec4 c = texture(coverage, vec2(x, z));
  return c.r;
}


bool expensive = true;


// FIXME: Final cloud density is not between 0.0 and 1.0. Why??
float sample_clouds(vec3 p){
  if (p.z > RENDER_DIST) return 0;
  // p = p + vec3(0, 0, time*10000);

  float cov = get_coverage(p);

  if (cov_only_in) return cov;

  float base_cloud = get_sample(p);

  float base_cloud_with_height = base_cloud;
  // float base_cloud_with_height = base_cloud*cumulus_function(get_height_fraction(p));
  float base_cloud_with_cov = remap(base_cloud_with_height, cov, 1.0, 0.0, 1.0);
  base_cloud_with_cov *= cov;

  base_cloud_with_cov = clamp(base_cloud_with_cov, 0.0, 1.0);

  if (!expensive)
  {
    float final_cloud = base_cloud_with_cov;
    return final_cloud * ( (RENDER_DIST - p.z) / RENDER_DIST);
  }
  
  float hd_mod = get_detail_sample(p);
  float final_cloud = remap(base_cloud_with_cov, hd_mod * 0.2, 1.0, 0.0, 1.0);

  return final_cloud * ( (RENDER_DIST - p.z) / RENDER_DIST);
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
  float steps = 4.;

  float STEP_SIZE = light_step_size;

  float density = 0.0;
  vec3 p = point + sun_dir*STEP_SIZE;

  for (int i = 0; i < steps ; i++) {
    density += sample_clouds(p);
    
    p += STEP_SIZE*sun_dir;
    if (p.y > CLOUDS_OUTER_SPHERE) return density;
  }
  return density;
}

vec3 sun_dir = normalize(vec3(0.0,-0.45,-0.8));
// vec3 sun_dir = normalize(vec3(0.3, 1, 0));
// vec3 sun_dir = normalize(vec3(sin(time*10), 0.5, cos(time*10)));
// vec3 sun_dir = normalize(vec3(0, sin(time*10), 0));

vec4 trace(vec3 origin, vec3 direction) {

  float ATM_START = PLANET_SPHERE + CLOUDS_INNER_SPHERE;
  float ATM_END = PLANET_SPHERE + CLOUDS_OUTER_SPHERE;


  float start_dist = ray_sphere_intersect(origin, direction, vec3(0, -PLANET_SPHERE, 0), ATM_START);

  float end_dist = ray_sphere_intersect(origin, direction, vec3(0, -PLANET_SPHERE, 0), ATM_END);
 
  // FIXME: More steps depending on distance. Fixed step size?
  float STEP_SIZE = (end_dist - start_dist) / num_steps_in;

  float T = 1.0;
  float lightEnergy = 1.0;

  float inScattering = 1.0;
  float tot_density = 0.0;
  // vec3 lightEnergy = vec3(0.0);
  vec3 color = vec3(0);
  vec3 bg = vec3(vec3(0.5, 0.6725, 1.0)*(1500.-gl_FragCoord.y)/resolution.y);

  float dist = start_dist;
  vec3 p = origin + direction*dist;

  int a = int(gl_FragCoord.x) % 4;
	int b = int(gl_FragCoord.y) % 4;
  p += direction*STEP_SIZE*bayerFilter[a * 4 + b];
  // p += direction*STEP_SIZE*hash(dot(direction, vec3(12.256, 2.646, 6.356)));

  for (int i = 0; i < num_steps_in ; i++) {
    // p += direction*(STEP_SIZE*(0.6+0.4*hash(dot(direction, vec3(12.256, 2.646, 6.356)))));
    p += direction*STEP_SIZE;

      float density = sample_clouds(p);
      if (density > 0.0) {

        float lightSamples = lightmarch(p, sun_dir);
        float dT = beer(density * STEP_SIZE/(100*trans));

        inScattering = 1;
        float forward_scattering = henyey_greenstein(p, p-sun_dir, 0.2);

        T *= dT;
        lightEnergy *= (beer(lightSamples)) * forward_scattering * inScattering / scat;
      }
    }
  color += 0.3;
  color += lightEnergy;
  // lightEnergy += forward_scattering * (STEP_SIZE / (end_dist - start_dist));
  // color += T;
  // TODO: Better alpha blending?
  return vec4(color*vec3(0.9, 0.95, 1.0), 1.0 - T*3);
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

float AMPLITUDE = 500;

vec4 terrain_march(vec3 origin, vec3 direction) {
  float STEP_SIZE = 20.0; 
  vec3 p = (origin + vec3(0, 0, time*1000)) + direction*STEP_SIZE;

  for (int i = 0; i < 200; i++) {
    float height = get_sample(p);

    if ((height * (-AMPLITUDE)) > p.y) {
      // return vec4(mix(vec3(0.2, 0.1, 0.0), vec3(0.4, 0.7, 0.3), (exp(-height*10))), 1.0);

      float ATM_START = PLANET_SPHERE + CLOUDS_INNER_SPHERE;
      float start_dist = ray_sphere_intersect(p, sun_dir, vec3(0, -PLANET_SPHERE, 0), ATM_START);
      p += sun_dir*start_dist;
      float march = lightmarch(p, sun_dir);
      return vec4(vec3(0.4, 0.7, 0.3), 1.0);
    }

    p += direction*STEP_SIZE;
  }
  return vec4(0);
}

void main() {

  vec2 uv=gl_FragCoord.xy/resolution.xy;
  // Offset, so center of screen is origin
  uv -= vec2(0.5);
  // Scale, so there is no rectangular distortion
  uv.x *= resolution.x/resolution.y; 
   
  vec3 orig=vec3(0.0,0.0,0.0);
  // vec3 look_at=vec3(sin(time), 0.2 + cos(time)/8, 1.0);
  vec3 look_at=vec3(0.0, 0.1, 1.0);
  float zoom=1.0;

  vec3 direction = getdir(uv, orig, look_at, zoom); 
    

  // vec2 coord = (2*gl_FragCoord.xy - resolution)/resolution.x;

  // vec3 direction = vec3(look_in, 1.0);
  // direction = normalize(vec3(direction.xy + coord, direction.z));

  vec3 origin = orig_in;

  if (direction.y > 0.0)
  {
    color = trace(origin, direction);
  }
  else
  {
    color = vec4(0);
    // color = terrain_march(origin, direction);
  }


  // Visualize textures
  // color = vec4(vec3(sample_clouds(vec3(gl_FragCoord.x, time*100, gl_FragCoord.y))), 1);
  // color = vec4(vec3(get_sample(vec3(gl_FragCoord.x, gl_FragCoord.y, time*100))), 1);
 }
