#include "icpCompute.h"
#include "scenes/GUIScene.h"

//----------------------------------------------------------------------------------------------------------
ICPCompute::ICPCompute(glm::vec3 origin, int resolution, float scale)
    : m_computeICPShader{}
    , m_texID{}
{
	m_computeICPShader.setupShaderFromFile(GL_COMPUTE_SHADER, "resources/computeSDF.comp");
	m_computeICPShader.linkProgram();

	setupTexture();
}

void ICPCompute::setupTexture()
{
	// dimensions of the image
	const int tex_w = 640, tex_h = 480;

	// Model
	glGenTextures(1, &m_texID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_texID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w, tex_h, 0, GL_RGBA, GL_FLOAT, NULL);
	glBindImageTexture(1, m_texID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
}

//----------------------------------------------------------------------------------------------------------
void ICPCompute::compute(unsigned int newVertexWorldTex)
{
	m_computeICPShader.begin();

	// m_computeICPShader.setUniform1f("_zeroPlaneDistInv", 1.0 / m_planeDist);
	// m_computeICPShader.setUniform1f("_zeroPixelSizeDouble", 2. * m_pixelSize);
	// glBindImageTexture(0, depthTexID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
	glBindImageTexture(1, newVertexWorldTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	m_computeICPShader.dispatchCompute(640, 480, 1);
	m_computeICPShader.end();
}

unsigned int ICPCompute::getTexID()
{
	return m_texID;
}
