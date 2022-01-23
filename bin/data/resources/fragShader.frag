
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
#define MAX_STEPS 100
#define MAX_DIST 100.
#define SURFACE_DIST .01

uniform sampler3D volume_tex;

float GetDist(vec3 p)
{
    vec4 s = vec4(0,0,0,.75); //Sphere. xyz is position w is radius
    vec3 c = vec3(2,2,2);
    
    vec3 q = p-c*clamp(round(p/c),vec3(-4, -4, -9), vec3(4, 4, -1));

    float sphereDist = length(q-s.xyz) - s.w;

    //float d = sphereDist;
    return sphereDist;
}
 
vec3 GetNormal(vec3 p)
{ 
    float d = GetDist(p); // Distance
    vec2 e = vec2(.01,0); // Epsilon
    vec3 n = d - vec3(
    GetDist(p-e.xyy),  
    GetDist(p-e.yxy),
    GetDist(p-e.yyx));
   
    return normalize(n);
}

float RayMarch(vec3 ro, vec3 rd) 
{
    float dO = 0.; //Distance Origin
    for(int i=0;i<MAX_STEPS;i++)
    {
        vec3 p = ro + rd * dO;
        float ds = clamp(GetDist(p), -0.1, 0.1); // ds is Distance Scene
        dO += ds;
        if(dO > MAX_DIST || ds < SURFACE_DIST) break;
    }
    return dO;
}

//Random number [0:1] without sine
#define HASHSCALE1 .1031
float hash(float p)
{
	vec3 p3  = fract(vec3(p) * HASHSCALE1);
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.x + p3.y) * p3.z);
}

float calcAO( in vec3 pos, in vec3 nor, in float maxDist, in float falloff )
{
	float ao = 0.0;
	const int nbIte = 6;
    for( int i=0; i<nbIte; i++ )
    {
        float l = hash(float(i))*maxDist;
        vec3 rd = nor*l;
        
        ao += (l - max( GetDist( pos + rd ),0.)) / maxDist * falloff;
    }
	
    return clamp( 1.-ao/float(nbIte), 0., 1.);
}

float GetDistSDF(vec3 p)
{
    // Transform world position to SDF Base
    vec3 relPos = (sdfBaseTransform * vec4(p, 1)).xyz; 

    if(relPos.x < 0 || relPos.y < 0 || relPos.z < 0 || relPos.x > 1 || relPos.y > 1|| relPos.z > 1)
    {
        return 1.0f;
    }
    
    // Replace by uniform
    return texture(volume_tex, relPos).r;
}

vec2 RayMarchSDF(vec3 ro, vec3 rd) 
{
    // minDist first step
    const vec3 b = vec3(10, 10, 10);
    vec3 q = abs(ro-vec3(0, 0, -10)) - b;
    float distanceBox = length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);

    float dO = distanceBox; //Distance Origin

    int i = 0;
    for(i = 0; i<MAX_STEPS;i++)
    {
        vec3 p = ro + rd * dO;
        float ds = GetDistSDF(p); // ds is Distance Scene
        dO += ds;
        if(dO > MAX_DIST || ds < SURFACE_DIST) break;
    }

    return vec2(dO, i * 1.0 / MAX_STEPS );
}

void main()
{
    vec3 ro = cameraWorld; // works for backface, too
    vec3 rd = normalize(world - cameraWorld);

    vec2 d = RayMarchSDF(ro, rd);

    /*
    float d = RayMarch(ro,rd); // Distance
    
    */
    //if(d.x < MAX_DIST)
    {
        // Hit point
        vec3 pos = ro + rd * d.x;
        vec3 nor = GetNormal(pos);
        vec3 ref = reflect(rd, nor);


        //float occ = calcAO( pos, nor, 10, .5);

        //vec3 posToWorld = (sdfBaseTransform * vec4(pos, 1)).xyz; 

        //vec3 color = texture(volume_tex, posToWorld).rgb;
        //vec3 color = posToWorld;
        
        // Cut out of bounds
        // if(posToWorld.x < 0 || posToWorld.y < 0 || posToWorld.z < 0 || posToWorld.x > 1 || posToWorld.y > 1|| posToWorld.z > 1)
        // {
        //     discard;
        //     //color = vec3(1,0,1);
        // }
        
        //float refl = RayMarch(pos + ref*0.1,ref); // Distance
        //vec3 reflPos = pos + d*ref;
        //vec3 reflNor = GetNormal(reflPos);
        //if(refl < MAX_DIST)
        //{
        //    color = color * vec3(0.2, 0.2, 0.2);
        //}
        
    
        outputColor = vec4(vec3(d.y), 1.0);
    }
   //else
   //{
   //    discard;
   //}
}