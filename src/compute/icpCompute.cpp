#include "icpCompute.h"
#include "scenes/GUIScene.h"

//----------------------------------------------------------------------------------------------------------
ICPCompute::ICPCompute()
    : m_computeICPShader{}
    , m_texID{}
{
	m_computeICPShader.setupShaderFromFile(GL_COMPUTE_SHADER, "resources/computeICP.comp");
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
void ICPCompute::compute(unsigned int newVertexWorldTex, glm::mat4x4& viewWorldIt, glm::mat4x4& viewProjectionIt)
{
	m_computeICPShader.begin();

	m_computeICPShader.setUniformMatrix4f("viewToWorldIt", viewWorldIt);
	m_computeICPShader.setUniformMatrix3f("viewToWorldItRot", viewWorldIt);
	m_computeICPShader.setUniformMatrix4f("viewProjectionIt", viewProjectionIt);

	// glBindImageTexture(0, depthTexID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
	glBindImageTexture(1, newVertexWorldTex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

	// correspondance
	glBindImageTexture(2, m_texID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	m_computeICPShader.dispatchCompute(640, 480, 1);
	m_computeICPShader.end();

	std::vector<glm::vec4> framedata(640 * 480);
	glBindTexture(GL_TEXTURE_2D, m_texID);
	glGetTextureImage(m_texID, 0, GL_RGBA, GL_FLOAT, framedata.size() * sizeof(glm::vec4), &framedata[0]);

	int test = 0;
}

unsigned int ICPCompute::getTexID()
{
	return m_texID;
}
