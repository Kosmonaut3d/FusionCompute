// vertex shader

#version 420

uniform mat4 modelMatrix;
uniform mat4 modelViewProjectionMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 textureMatrix;

in vec4 position;
out vec3 world;

void main(){
    gl_Position = modelViewProjectionMatrix * position;
    world = (modelMatrix * position).xyz;
}