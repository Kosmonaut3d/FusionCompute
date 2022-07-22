// fullscreenQuad.frag
// Render a texture by texcoords
#version 460 core

layout(binding = 0) uniform sampler2D t;

in vec2 texcoords;
out vec4 outputColor;

void main() {
	vec4 color = vec4(texture(t,vec2(texcoords.x, 1-texcoords.y)).rgb, 1);
	outputColor = vec4(color.rgb, 1);
}