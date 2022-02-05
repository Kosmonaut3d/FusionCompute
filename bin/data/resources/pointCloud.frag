#version 440

out vec4 outputColor;
layout(binding=1) uniform sampler2D colorTex;

in vec2 texcoords; // texcoords are in the normalized [0,1] range for the viewport-filling quad part of the triangle

void main() {
   vec3 tex = texture2D(colorTex,texcoords).rgb;
   outputColor =  mix(vec4(1,0,0,1), vec4(tex, 1), 0.5);
}