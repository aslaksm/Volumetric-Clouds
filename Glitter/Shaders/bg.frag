#version 430 core

uniform layout (location = 4) vec2 resolution;
out vec4 color;

void main() {
  color = vec4(vec3(0.5, 0.6725, 1.0)*(1500.-gl_FragCoord.y)/resolution.y, 1.0);
}
