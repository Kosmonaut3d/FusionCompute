// pointCloud.vert
// This vertex shader will read the point cloud texture and transform vertices to the position stored there

#version 460 core

layout(rgba32f, binding = 0) uniform readonly image2D worldTex;
uniform sampler2D colorTex;

uniform mat4 modelViewProjectionMatrix;

const float WIDTH = 640.0f;
const float HEIGHT = 480.0f;

const double WInv = 1.0 / WIDTH;
const double HInv = 1.0 / HEIGHT;

out vec2 texcoords; // texcoords are in the normalized [0,1] range for the viewport-filling quad part of the triangle

void main() {
        // Get 2D position by remapping 1D gl_VertexID
        float x = mod(gl_VertexID, WIDTH);
        float y = floor(gl_VertexID) / WIDTH;
        ivec2 pixel_coord = ivec2(mod(gl_VertexID, WIDTH), floor(gl_VertexID) / WIDTH);
        
        texcoords = pixel_coord*vec2(WInv,HInv);

        gl_Position = modelViewProjectionMatrix * vec4(imageLoad(worldTex, pixel_coord).xyz, 1);
}