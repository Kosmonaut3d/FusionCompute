#version 420

layout(binding = 0) uniform sampler2DRect t;

in vec2 texcoords;
out vec4 outputColor;

void main() {
   vec4 color1 = vec4(texture(t,texcoords).rgb, 1); // vec4(texcoords,1, 1);//
   outputColor = color1;
   //outputColor =  mix(vec4(texcoords,0,1), color1, 0.9);
}