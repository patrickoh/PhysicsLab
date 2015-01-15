uniform mat4 modelview;
uniform mat4 proj;

layout(location = 0) in vec4 position;

void main()
{
	vec4 eyePos = modelview * gl_Vertex;
    gl_Position = proj * eyePos;

	float dist = length(eyePos.xyz);
    float att = inversesqrt(0.1f*dist);
    gl_PointSize = 2.0f * att;
}