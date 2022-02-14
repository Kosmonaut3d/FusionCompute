#include "sdfCompute.h"
#include "scenes/GUIScene.h"

//----------------------------------------------------------------------------------------------------------
SDFCompute::SDFCompute(glm::vec3 origin, int resolution, float scale)
    : m_computeSDFShader{}
    , m_texID{}
    , m_resolution{resolution}
    , m_origin{origin}
    , m_scale{scale}
{
	setupTexture();
}

void SDFCompute::setupTexture()
{
	glGenTextures(1, &m_texID);
	glBindTexture(GL_TEXTURE_3D, m_texID);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, m_resolution, m_resolution, m_resolution, 0, GL_RED, GL_FLOAT, NULL);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RG32F, m_resolution, m_resolution, m_resolution, 0, GL_RG, GL_FLOAT, NULL);
}

//----------------------------------------------------------------------------------------------------------
void SDFCompute::compute(unsigned int pointCloudId, unsigned int pointCloudNormalId)
{

}

//----------------------------------------------------------------------------------------------------------
unsigned int SDFCompute::getTextureID()
{
	return m_texID;
}

//----------------------------------------------------------------------------------------------------------
void SDFCompute::drawOutline()
{
	ofPushStyle();
	ofGetCurrentRenderer()->setFillMode(ofFillFlag::OF_OUTLINE);
	ofSetColor(200, 100, 200);
	float scalehalf = m_scale / 2;
	ofDrawBox(m_origin + glm::vec3(scalehalf, scalehalf, scalehalf), m_scale, m_scale, m_scale);
	ofPopStyle();
}
