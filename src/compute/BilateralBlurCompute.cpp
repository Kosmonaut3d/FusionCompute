#include "BilateralBlurCompute.h"
#include "scenes/GUIScene.h"

//----------------------------------------------------------------------------------------------------------
BilateralBlurCompute::BilateralBlurCompute()
    : m_computeBlurShader{}
    , m_texID{}
{
	m_computeBlurShader.setupShaderFromFile(GL_COMPUTE_SHADER, "resources/computeBilateralBlur.comp");
	m_computeBlurShader.linkProgram();

	// dimensions of the image
	const int tex_w = 640, tex_h = 480;

	// Model
	glGenTextures(1, &m_texID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_texID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, tex_w, tex_h, 0, GL_RED, GL_FLOAT, NULL);
	glBindImageTexture(1, m_texID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
}

//----------------------------------------------------------------------------------------------------------
void BilateralBlurCompute::compute(ofTexture& depthImage, bool blur)
{
	GLuint query;

	if (GUIScene::s_measureTime)
	{
		glGenQueries(1, &query);
		glBeginQuery(GL_TIME_ELAPSED, query);
	}

	// Worlds
	m_computeBlurShader.begin();

	m_computeBlurShader.setUniform1i("blur", blur);
	glBindImageTexture(0, depthImage.getTextureData().textureID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R16UI);
	glBindImageTexture(1, m_texID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);

	// Thread size = 32, 16 -> 20*32 = 640, 30*16 = 480
	m_computeBlurShader.dispatchCompute(20, 30, 1);
	m_computeBlurShader.end();

	if (GUIScene::s_measureTime)
	{
		glEndQuery(GL_TIME_ELAPSED);
		glGetQueryObjectui64v(query, GL_QUERY_RESULT, &GUIScene::s_bilateralBlur_measureComputeTime);
	}
}

//----------------------------------------------------------------------------------------------------------
unsigned int BilateralBlurCompute::getTextureID()
{
	return m_texID;
}
