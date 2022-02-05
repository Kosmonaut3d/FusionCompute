#version 420 core

in vec2 texcoords;
layout(binding = 0) uniform sampler2D t;
out vec4 outputColor;

void main() {
   vec4 color1 = vec4(texture(t,texcoords).rgb, 1); // vec4(texcoords,1, 1);//
   //outputColor = 
   outputColor =  mix(vec4(texcoords,0,1), color1, 0.9);
}