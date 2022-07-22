// fragment shader for the 2D slice
// This shader will visualize a SDF by slicing through in a 2D plane

#version 460 core

out vec4 outputColor;
in vec3 world;
uniform mat4 sdfBaseTransform;

uniform sampler3D tex3D;

void main()
{
    vec3 relPos = (sdfBaseTransform * vec4(world, 1)).xyz; 
	
	vec2 valSDF = texture(tex3D, relPos).rg;

	if(valSDF.x < .0)
	{
		outputColor =  vec4(-valSDF.x,0, 0 ,1.);
	}
	else
	{
		outputColor =  vec4(0, valSDF.x, 0 ,1.);
	}
}