#version 330 core

in vec2 texcoords;
uniform sampler2D t;
out vec4 outputColor;

void main() {
   vec4 color1 = texture2D(t,texcoords); // vec4(texcoords,1, 1);//
   outputColor = color1;
}