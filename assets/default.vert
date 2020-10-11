#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform CameraUniform {
    mat4 camera;
};

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 color;

void main() {
	gl_Position = camera * vec4(inPosition, 1.0);
	color = normalize(inPosition);
}