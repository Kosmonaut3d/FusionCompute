
// fragment shader

#version 150

out vec4 outputColor;
in vec3 world;
uniform float cameraFar;
uniform vec3 cameraWorld;
uniform vec3 sdfOrigin;

#define PI 3.1415925359
#define TWO_PI 6.2831852
#define MAX_STEPS 100
#define MAX_DIST 100.
#define SURFACE_DIST .01

uniform sampler3D volume_tex;

float sdTorus( vec3 p, vec2 t )
{
  vec2 q = vec2(length(p.xz)-t.x,p.y);
  return length(q)-t.y;
}

float GetDist(vec3 p)
{
    vec4 s = vec4(0,0,-10,3); //Sphere. xyz is position w is radius
    float sphereDist = length(p-s.xyz) - s.w;
    float torusDist = sdTorus(p - vec3(0, -2, -10), vec2(10,0.2));
    //float planeDist = p.y+3;
    float d = min(sphereDist,torusDist);
    //float d = sphereDist;
    return d;
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
        float ds = GetDist(p); // ds is Distance Scene
        dO += ds;
        if(dO > MAX_DIST || ds < SURFACE_DIST) break;
    }
    return dO;
}

float calcAO( in vec3 pos, in vec3 nor )
{
	float occ = 0.0;
    float sca = 1.0;
    for( int i=0; i<5; i++ )
    {
        float h = 0.01 + 0.42*float(i)/4.0;
        float d = clamp(GetDist( pos + h*nor ), 0.0, h);
        occ += (h-d)*sca;
        sca *= 0.8;
        if( occ>0.30 ) break;
    }
    return clamp( 1.0 - 3.0*occ, 0.0, 1.0 ) * (0.5+0.5*nor.y);
}

void main()
{
    vec3 ro = cameraWorld; // works for backface, too
    vec3 rd = normalize(world - cameraWorld);

    float windowWidth = 1920.0;
    float windowHeight = 1080.0;

    float d = RayMarch(ro,rd); // Distance

    if(d < MAX_DIST)
    {
        // Hit point
        vec3 pos = ro + rd * d;
        vec3 nor = GetNormal(pos);
        vec3 ref = reflect(rd, nor);

        float refl = RayMarch(pos + ref*0.1,ref); // Distance
        vec3 reflPos = pos + d*ref;
        vec3 reflNor = GetNormal(reflPos);

        float occ = calcAO( pos, nor );

        vec3 color = texture(volume_tex, vec3(0,0,0)).rgb;
        if(refl < MAX_DIST)
        {
            color = color * vec3(0.2, 0.2, 0.2);
        }
        

        // TODO: We could add a correct gl_FragDepth for fun
    
        outputColor = vec4(color * occ, 1.0);
    }
    else
    {
        discard;
    }
}