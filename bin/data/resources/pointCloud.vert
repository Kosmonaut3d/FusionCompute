#version 420

uniform sampler2D worldTex;
uniform sampler2D colorTex;

uniform mat4 modelViewProjectionMatrix;
uniform float mixFactor;

const float WIDTH = 640;
const float HEIGHT = 480;

out vec2 texcoords; // texcoords are in the normalized [0,1] range for the viewport-filling quad part of the triangle

void main() {
        vec2 pixel_coord = vec2(mod(gl_VertexID, WIDTH), gl_VertexID / WIDTH);
        
        texcoords = pixel_coord/vec2(WIDTH,HEIGHT);

        gl_Position = modelViewProjectionMatrix * vec4(texture(worldTex, texcoords).xyz * mixFactor, 1); // vec4(vec3(texcoords, 0) * mixFactor, 1); //
}