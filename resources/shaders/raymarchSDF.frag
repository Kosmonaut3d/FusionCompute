// rayMarchSDFColor
//
// This fragment shader tries to march through an SDF 
// It will display either the normal or a mesh shading, depending on _drawNormals.

#version 460 core

out vec4 outputColor;
in vec3 world;
uniform vec3 cameraWorld;
uniform mat4 sdfBaseTransform;
uniform float sdfResolution;

uniform mat4 viewprojection;

uniform float _truncationDistance;

uniform int _drawNormals;

#define MAX_STEPS 30
#define MAX_DIST 8
#define SURFACE_DIST .01

uniform sampler3D signedDistanceVolumeTexture;

// Header
float getSignedDistance(vec3 p);
vec2 rayMarchSDF(vec3 ro, vec3 rd);


// Calculates the normal numerically.
// Equivalent to the gradient at that point
vec3 GetNormal(vec3 p)
{ 
    float d = getSignedDistance(p); // middle sample
    vec2 e = vec2(.01,0); // Epsilon
    vec3 n = d - vec3(
    getSignedDistance(p-e.xyy),  
    getSignedDistance(p-e.yxy),
    getSignedDistance(p-e.yyx));
    return normalize(n);
}

// The exact distance to the actual SDF volume
// https://iquilezles.org/articles/distfunctions/
float getDistanceSDFVolume(vec3 p)
{
    const vec3 b = vec3(2, 2, 2);
    vec3 q = abs(p-vec3(0, 0, -2)) - b;
    return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}


// Returns the signed distance, if still inside the volume, or MAX_DIST
float getSignedDistance(vec3 p)
{
    // Transform world position to SDF Base transformation
    vec3 relPos = (sdfBaseTransform * vec4(p, 1)).xyz; 

    // SDF base coordinates are such that values <0 or >1 are outside the field
    if(relPos.x < 0 || relPos.y < 0 || relPos.z < 0 || relPos.x > 1 || relPos.y > 1|| relPos.z > 1)
    {
        // If outside -> do not trace further
        return MAX_DIST;
    }
    
    // Trilinearly interpolated SD value
    return texture(signedDistanceVolumeTexture, relPos).r;
}

// step / march from r0 in the direction of rd.
// returns: vec2, x = distance, y = iterations/MAX_STEPS
vec2 rayMarchSDF(vec3 ro, vec3 rd) 
{
    // d0 = distance marched so far.
    float dO = 0;
    int i = 0;
    for(i = 0; i<MAX_STEPS;i++)
    {
        // Advance the step
        vec3 p = ro + rd * dO;

        // get new TSD value
        float ds = getSignedDistance(p); 

        // Advance by ds
        dO += ds;
        
        // Check if we are either exceeding maximum distance or have hit something
        if(dO > (MAX_DIST) || abs(ds) < SURFACE_DIST) break;
    }
    
    // Cast to double by multiplication with 1.0
    return vec2(dO, i * 1.0 / MAX_STEPS );
}

// Starting point for this shader
void main()
{
    vec3 ro = cameraWorld; // works for backface, too
    vec3 rd = normalize(world - cameraWorld);
    
    float dInit = 0.001;

    vec2 d = rayMarchSDF(ro+rd*dInit, rd);

    if(d.x < MAX_DIST)
    {
        // Hit point
        vec3 pos = ro + rd * d.x;
        vec3 nor = GetNormal(pos);
        
        if(_drawNormals > 0)
        {
            // beautify normal by remapping from [-1,1] to [0,1]
            nor = (nor + vec3(1)) * .5;
            outputColor = vec4( nor, 1.0);
        }
        else
        {
            // simple phong shading, light comes from the camera
            float ndl = dot(nor, -rd);
            outputColor = vec4(ndl.xxx, 1.0);
        }
    }
    else
    { 
        discard;
    }
}