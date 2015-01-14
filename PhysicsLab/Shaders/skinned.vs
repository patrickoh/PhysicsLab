#version 400

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec2 texture_coord;
layout (location = 3) in ivec4 bone_id;
layout (location = 4) in vec4 Weights;

const int MAX_BONES = 32;

uniform mat4 mvpMatrix;
uniform mat4 boneMatrices[MAX_BONES];

out vec3 normal;
out vec2 texCoord;
out vec4 colour;

void main()
{
	vec4 Vertex = vec4(vertex_position.x, vertex_position.y, vertex_position.z, 1.0);

	mat4 BoneTransform = boneMatrices[bone_id[0]] * Weights[0];
    BoneTransform     += boneMatrices[bone_id[1]] * Weights[1];
    BoneTransform     += boneMatrices[bone_id[2]] * Weights[2];
    BoneTransform     += boneMatrices[bone_id[3]] * Weights[3];

	gl_Position = mvpMatrix * BoneTransform * Vertex;

	colour = vec4 (0.0, 0.0, 0.0, 0.5);
	
	texCoord = texture_coord;
	normal = vertex_normal;
}