#version 330

uniform mat4 view;
uniform mat4 proj;

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec4 vertex_colour;

void main()
{
	vec4 Vertex = vec4(vertex_position.x, vertex_position.y, vertex_position.z, 1.0);
	vec4 eyePos = view * Vertex;
    gl_Position = proj * eyePos;

	float dist = length(eyePos.xyz);
    float att = inversesqrt(0.1f*dist);
    gl_PointSize = 2.0f * att;

	//outColour = vertex_colour;
}