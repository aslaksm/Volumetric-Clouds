#version 430 core


in layout(location = 0) vec3 position;
in layout(location = 1) vec3 normal_in;
in layout(location = 2) vec2 textureCoordinates_in;

uniform layout (location = 5) float time;

void main()
{
    gl_Position = vec4(position, 1.0);
}
