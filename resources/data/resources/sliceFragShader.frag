
// fragment shader

#version 420

out vec4 outputColor;
in vec3 world;
uniform mat4 sdfBaseTransform;

//uniform sampler2D tex2D;
uniform sampler3D tex3D;

void main()
{
    vec3 relPos = (sdfBaseTransform * vec4(world, 1)).xyz; 
	
	float valSDF = texture(tex3D, relPos).r;

	if(valSDF < .4)
	{
		outputColor =  vec4(max(-valSDF*1000, 0.), valSDF.xx ,1.);
	}
	else
	{
		discard;
	}

	//vec3 valCompute =  texture(tex2D, relPos.zy).rbg;
}
/*
#define SURFACE_DIST .01

uniform sampler3D volume_tex;
uniform sampler2D tex2D;

void main()
{
    vec3 relPos = (sdfBaseTransform * vec4(world, 1)).xyz; 
	float value = texture(volume_tex, relPos).r;
	if(abs(value) < 0.01)
	{
		outputColor = vec4(1, 0, 0, 1);
	}
	else if(value < 0)
	{
		outputColor = vec4(0,vec2(-value), 1.0);
	}
	else
	{
		outputColor =  texture2D(tex2D, relPos.zy);
	}
}
*/