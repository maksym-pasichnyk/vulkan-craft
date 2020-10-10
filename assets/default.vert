#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform CameraUniform {
    mat4 camera;
};

layout(location = 0) in vec2 inPosition;

void main() {
	gl_Position = camera * vec4(inPosition.x, inPosition.y, 0.0, 1.0);
}