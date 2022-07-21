#version 460 core

out vec4 outputColor;
uniform sampler2D colorTex;

in vec2 texcoords; // texcoords are in the normalized [0,1] range for the viewport-filling quad part of the triangle

void main() {
   vec3 tex = texture(colorTex,texcoords).rgb;
   outputColor =  vec4(tex, 1) ;
}
