#include "PointCloudCompute.h"
#include "scenes/GUIScene.h"

//---------------------------------------------------
PointCloudComp::PointCloudComp()
    : m_planeDist(120)
    , m_pixelSize(.104f)
{
	m_computeModelShader.setupShaderFromFile(GL_COMPUTE_SHADER, "shaders/computeModelPCL.comp");
	m_computeModelShader.linkProgram();

	m_computeNormalShader.setupShaderFromFile(GL_COMPUTE_SHADER, "shaders/computeNormalPCL.comp");
	m_computeNormalShader.linkProgram();

	setUpOutputTexture();
}

//----------------------------------------------------------------------------------------------------------
void PointCloudComp::setUpOutputTexture()
{
	// dimensions of the image
	const int tex_w = 640, tex_h = 480;

	// Model
	for (int i = 0; i <= 1; i++)
	{
		// Create textures for 2 frames (needed for flip comparison)
		unsigned int* texModelId  = i == 0 ? &m_texModelID_0 : &m_texModelID_1;
		unsigned int* texNormalId = i == 0 ? &m_texNormalID_0 : &m_texNormalID_1;

		glGenTextures(1, texModelId);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, *texModelId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w, tex_h, 0, GL_RGBA, GL_FLOAT, NULL);
		glBindImageTexture(1, *texModelId, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

		// Normal map
		glGenTextures(1, texNormalId);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, *texNormalId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w, tex_h, 0, GL_RGBA, GL_FLOAT, NULL);
		glBindImageTexture(2, *texNormalId, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	}
}

//----------------------------------------------------------------------------------------------------------
void PointCloudComp::compute(unsigned int depthTexID, bool isFrame0)
{
	unsigned int texModelId  = isFrame0 ? m_texModelID_0 : m_texModelID_1;
	unsigned int texNormalId = isFrame0 ? m_texNormalID_0 : m_texNormalID_1;

	GLuint query;
	if (GUIScene::s_measureTime)
	{
		glGenQueries(1, &query);
		glBeginQuery(GL_TIME_ELAPSED, query);
	}

	// Worlds
	m_computeModelShader.begin();

	m_computeModelShader.setUniform1f("_zeroPlaneDistInv", 1.0 / m_planeDist);
	m_computeModelShader.setUniform1f("_zeroPixelSizeDouble", 2. * m_pixelSize);
	glBindImageTexture(0, depthTexID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
	glBindImageTexture(1, texModelId, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	// Resolution 640*480 / Threadsize 32, 16 =
	m_computeModelShader.dispatchCompute(20, 30, 1);
	m_computeModelShader.end();

	// Normals
	m_computeNormalShader.begin();
	glBindImageTexture(1, texModelId, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(2, texNormalId, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	m_computeModelShader.dispatchCompute(20, 30, 1);
	m_computeNormalShader.end();

	if (GUIScene::s_measureTime)
	{
		glEndQuery(GL_TIME_ELAPSED);
		glGetQueryObjectui64v(query, GL_QUERY_RESULT, &GUIScene::s_PCL_GPU_measuredComputeTime);
	}
}

//----------------------------------------------------------------------------------------------------------
unsigned int PointCloudComp::getModelTextureID(bool isFrame0)
{
	return isFrame0 ? m_texModelID_0 : m_texModelID_1;
}

//----------------------------------------------------------------------------------------------------------
unsigned int PointCloudComp::getNormalTextureID(bool isFrame0)
{
	return isFrame0 ? m_texNormalID_0 : m_texNormalID_1;
}

//----------------------------------------------------------------------------------------------------------
void PointCloudComp::registerKinectData(float planeDist, float pixelSize)
{
	m_planeDist = planeDist;
	m_pixelSize = pixelSize;
}
