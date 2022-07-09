#include "PointCloudCompute.h"
#include "scenes/GUIScene.h"

//---------------------------------------------------
PointCloudComp::PointCloudComp()
    : m_planeDist(120)
    , m_pixelSize(.104f)
{
	int result;
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &result);

	ofLogNotice() << "max computer work groups(x): " << result;

	int work_grp_cnt[3];

	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_grp_cnt[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_grp_cnt[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_grp_cnt[2]);

	printf("max global (total) work group counts x:%i y:%i z:%i\n", work_grp_cnt[0], work_grp_cnt[1], work_grp_cnt[2]);

	int work_grp_size[3];

	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &work_grp_size[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &work_grp_size[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &work_grp_size[2]);

	printf("max local (in one shader) work group sizes x:%i y:%i z:%i\n", work_grp_size[0], work_grp_size[1],
	       work_grp_size[2]);

	m_computeModelShader.setupShaderFromFile(GL_COMPUTE_SHADER, "resources/computeModelPCL.comp");
	m_computeModelShader.linkProgram();

	m_computeNormalShader.setupShaderFromFile(GL_COMPUTE_SHADER, "resources/computeNormalPCL.comp");
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

	m_computeModelShader.dispatchCompute(640, 480, 1);
	m_computeModelShader.end();

	// glMemoryBarrier(GL_ALL_BARRIER_BITS);

	// Normals
	m_computeNormalShader.begin();
	glBindImageTexture(1, texModelId, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(2, texNormalId, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	// Do not compute the last row, since a fetch outside is super expensive
	m_computeNormalShader.dispatchCompute(639, 479, 1);
	m_computeNormalShader.end();
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

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
