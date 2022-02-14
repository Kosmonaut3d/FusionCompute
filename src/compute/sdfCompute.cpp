#include "sdfCompute.h"
#include "scenes/GUIScene.h"

//----------------------------------------------------------------------------------------------------------
SDFCompute::SDFCompute()
    : m_computeSDFShader{}, m_texID{}
{
}

//----------------------------------------------------------------------------------------------------------
void SDFCompute::compute(unsigned int pointCloudId, unsigned int pointCloudNormalId)
{
}

//----------------------------------------------------------------------------------------------------------
unsigned int SDFCompute::getTextureID()
{
	return 0;
}
