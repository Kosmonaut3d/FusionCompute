
// fragment shader

#version 430

out vec4 outputColor;
in vec3 world;
uniform vec3 cameraWorld;
uniform mat4 sdfBaseTransform;
uniform float sdfResolution;

uniform mat4 viewprojection;
uniform float near;
uniform float far;

#define PI 3.1415925359
#define TWO_PI 6.2831852
#define MAX_STEPS 200
#define MAX_DIST 6
#define SURFACE_DIST .01

uniform sampler3D volume_tex;

// Header
float GetDistSDF(vec3 p);
vec2 RayMarchSDF(vec3 ro, vec3 rd);

 
vec3 GetNormal(vec3 p)
{ 
    float d = GetDistSDF(p); // Distance
    vec2 e = vec2(.01,0); // Epsilon
    vec3 n = d - vec3(
    GetDistSDF(p-e.xyy),  
    GetDistSDF(p-e.yxy),
    GetDistSDF(p-e.yyx));
   
    return normalize(n);
}

//Random number [0:1] without sine
#define HASHSCALE1 .1031
float hash(float p)
{
	vec3 p3  = fract(vec3(p) * HASHSCALE1);
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.x + p3.y) * p3.z);
}

float getDistanceSDFVolume(vec3 p)
{
    const vec3 b = vec3(2, 2, 2);
    vec3 q = abs(p-vec3(0, 0, -2)) - b;
    return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}

float GetDistSDF(vec3 p)
{
    // IDEA: Transform the p first to sdfBase and trace from there
    // Transform world position to SDF Base
    vec3 relPos = (sdfBaseTransform * vec4(p, 1)).xyz; 

    if(relPos.x < 0 || relPos.y < 0 || relPos.z < 0 || relPos.x > 1 || relPos.y > 1|| relPos.z > 1)
    {
        return .1;
    }

    // Replace by uniform
    return texture(volume_tex, relPos).r;
}

vec2 RayMarchSDF(vec3 ro, vec3 rd) 
{
    float dO = 0;
    int i = 0;
    for(i = 0; i<MAX_STEPS;i++)
    {
        vec3 p = ro + rd * dO;
        float ds = GetDistSDF(p); // ds is Distance Scene
        dO += ds;
        if(dO > (MAX_DIST) || ds < SURFACE_DIST) break;
    }

    return vec2(dO, i * 1.0 / MAX_STEPS );
}

void main()
{
    vec3 ro = cameraWorld; // works for backface, too
    vec3 rd = normalize(world - cameraWorld);

    float dInit = max(getDistanceSDFVolume(ro), 0.0); //max(distanceBox, 0.0); //Distance 
    vec2 d = RayMarchSDF(ro+rd*dInit, rd);
    
    if(d.x < MAX_DIST)
    {
        // Hit point
        //vec3 pos = ro + rd * d.x;
        //vec3 nor = GetNormal(pos);
    
        outputColor = vec4( d.yyy, 1.0);
    }
    else
    { 
        //outputColor = vec4( d.y, 0, 0 , 1.0);
        discard;
    }
}