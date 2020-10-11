#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform CameraUniform {
    mat4 camera;
};

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec3 vertex;
layout(location = 1) out vec3 normal;

void main() {
	gl_Position = camera * vec4(inPosition, 1.0);

	vertex = inPosition;
	normal = inNormal;
}