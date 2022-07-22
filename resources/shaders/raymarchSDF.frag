// raymarchSDFColor
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
    vec3 q = abs(p-vec3(0, 0, -1)) - b;
    return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}


vec4 resampleGradientAndDensity(vec3 position)
{
    float sample_point = texture(volume_tex, position).r;
    vec3 size = (512).xxx;
    vec3 step = 1.0 / size;
    vec3 sample0, sample1;
    sample0.x = texture(volume_tex,
    vec3(position.x - step.x, position.y, position.z)).r;
    sample1.x = texture(volume_tex,
    vec3(position.x + step.x, position.y, position.z)).r;
    sample0.y = texture(volume_tex,
    vec3(position.x, position.y - step.y, position.z)).r;
    sample1.y = texture(volume_tex,
    vec3(position.x, position.y + step.y, position.z)).r;
    sample0.z = texture(volume_tex,
    vec3(position.x, position.y, position.z - step.z)).r;
    sample1.z = texture(volume_tex,
    vec3(position.x, position.y, position.z + step.z)).r;
    vec3 scaledPosition = position * size - 0.5;
    vec3 fraction = scaledPosition - floor(scaledPosition);
    vec3 correctionPolynomial = (fraction * (fraction - 1.0)) / 2.0;
    sample_point += dot((sample0 - sample_point * 2.0 + sample1),
    correctionPolynomial);
    return vec4(normalize(sample1 - sample0), sample_point);
}

float GetDistSDF(vec3 p)
{
    /*
    float dist = getDistanceSDFVolume(p);
    if(dist > 0)
    {
        discard;
    }*/

    // IDEA: Transform the p first to sdfBase and trace from there
    // Transform world position to SDF Base
    vec3 relPos = (sdfBaseTransform * vec4(p, 1)).xyz; 

    if(relPos.x < 0 || relPos.y < 0 || relPos.z < 0 || relPos.x > 1 || relPos.y > 1|| relPos.z > 1)
    {
        return 10;
    }
    
    return texture(volume_tex, relPos).r;
    //return resampleGradientAndDensity(relPos).w;
}

vec2 RayMarchSDF_KiFu(vec3 ro, vec3 rd) 
{
    float dO = 0;
    int i = 0;
    float ds_old = 0;
    float step_size = 0.02;
    float step_it = step_size;
    for(i = 0; i<MAX_STEPS;i++)
    {
        vec3 p = ro + rd * dO;
        float ds = GetDistSDF(p); // ds is Distance Scene
        
        if(ds < 0)
        {
            dO = dO - step_size * ds_old / (ds - ds_old);
            return vec2(dO, i * 1.0 / MAX_STEPS );
        }

        if(ds >= _truncationDistance)
        {
            step_it = _truncationDistance;
        }
        else
        {
            step_it = step_size;
        }

        ds_old = ds;
        
        dO += step_it;
        if(dO > (MAX_DIST) || (ds) < SURFACE_DIST) 
        {
            return vec2(dO, i * 1.0 / MAX_STEPS );
        }
    }

    return vec2(MAX_DIST, i * 1.0 / MAX_STEPS );
}

vec2 RayMarchSDF(vec3 ro, vec3 rd) 
{
    float dO = 0;
    int i = 0;
    for(i = 0; i<MAX_STEPS;i++)
    {
        vec3 p = ro + rd * dO;
        float ds = GetDistSDF(p); // ds is signed distance

        dO += ds;
        if(dO > (MAX_DIST) || abs(ds) < SURFACE_DIST) break;
    }

    return vec2(dO, i * 1.0 / MAX_STEPS );
}

void main()
{
    vec3 ro = cameraWorld; // works for backface, too
    vec3 rd = normalize(world - cameraWorld);
    
    /*if(getDistanceSDFVolume(cameraWorld) > 0)
    {
        ro = world;
    }*/
    

    float dInit = 0.001;

    vec2 d = RayMarchSDF(ro+rd*dInit, rd);

    if(d.x < MAX_DIST)
    {
        // Hit point
        vec3 pos = ro + rd * d.x;
        vec3 nor = GetNormal(pos);
        
        if(_drawNormals > 0)
        {
        // beautify normal
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
        //outputColor = vec4( d.y, 0, 0 , 1.0);
        discard;
    }
}