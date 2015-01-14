#version 330

//This is the layout convention accross all shader files
layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec2 texture_coord;

uniform mat4 mvpMatrix;

out vec4 colour;
out vec3 normal;
out vec2 texCoord;

void main()
{
	vec4 Vertex = vec4(vertex_position.x, vertex_position.y, vertex_position.z, 1.0);
	gl_Position = mvpMatrix * Vertex;

	texCoord = texture_coord;
	normal = vertex_normal;
}