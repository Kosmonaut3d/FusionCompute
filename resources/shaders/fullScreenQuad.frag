#version 460 core

layout(binding = 0) uniform sampler2D t;

in vec2 texcoords;
out vec4 outputColor;

void main() {
	vec4 color1 = vec4(texture(t,vec2(texcoords.x, 1-texcoords.y)).rgb, 1); // vec4(texcoords,1, 1);//

	/*
	vec2 uv = texcoords;
	uv *=  1.0 - texcoords.yx;   //vec2(1.0)- uv.yx; -> 1.-u.yx; Thanks FabriceNeyret !
	float vig = uv.x*uv.y * 15.0; // multiply with sth for intensity
	vig = pow(vig, 0.45); // change pow for modifying the extend of the  vignette
	*/
	outputColor = vec4(color1.rgb, 1);
}