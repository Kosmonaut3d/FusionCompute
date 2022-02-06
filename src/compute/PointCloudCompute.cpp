#include "PointCloudCompute.h"
#include "scenes/GUIScene.h"

//---------------------------------------------------
PointCloudComp::PointCloudComp()
{
	int result;
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &result);

	ofLogNotice() << "max computer work groups(x): " << result;

	int work_grp_cnt[3];

	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_grp_cnt[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_grp_cnt[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_grp_cnt[2]);

	printf("max global (total) work group counts x:%i y:%i z:%i\n",
		work_grp_cnt[0], work_grp_cnt[1], work_grp_cnt[2]);

	int work_grp_size[3];

	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &work_grp_size[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &work_grp_size[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &work_grp_size[2]);

	printf("max local (in one shader) work group sizes x:%i y:%i z:%i\n",
		work_grp_size[0], work_grp_size[1], work_grp_size[2]);

	m_computeModelShader.setupShaderFromFile(GL_COMPUTE_SHADER, "resources/computeModelPCL.comp");
	m_computeModelShader.linkProgram();

	m_computeNormalShader.setupShaderFromFile(GL_COMPUTE_SHADER, "resources/computeNormalPCL.comp");
	m_computeNormalShader.linkProgram();

	setUpOutputTexture();
}

//---------------------------------------------------
void PointCloudComp::setUpOutputTexture()
{
	// dimensions of the image
	const int tex_w = 640, tex_h = 480;

	// Model
	glGenTextures(1, &m_texModelID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_texModelID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w, tex_h, 0, GL_RGBA, GL_FLOAT, NULL);
	glBindImageTexture(1, m_texModelID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	// Normal map
	glGenTextures(1, &m_texNormalID);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_texNormalID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w, tex_h, 0, GL_RGBA, GL_FLOAT, NULL);
	glBindImageTexture(2, m_texNormalID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
}

//---------------------------------------------------
void PointCloudComp::compute(ofTexture & depthImage)
{
	// Worlds
	m_computeModelShader.begin();

	m_computeModelShader.setUniform1f("_zeroPlaneDist", m_planeDist);
	m_computeModelShader.setUniform1f("_zeroPixelSize", m_pixelSize);
	glBindImageTexture(0, depthImage.getTextureData().textureID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R16);

	m_computeModelShader.dispatchCompute(640, 480, 1);
	m_computeModelShader.end();

	//glMemoryBarrier(GL_ALL_BARRIER_BITS);

	// Normals
	m_computeNormalShader.begin();
	glBindImageTexture(1, m_texModelID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(2, m_texNormalID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	m_computeNormalShader.dispatchCompute(640, 480, 1);
	m_computeNormalShader.end();
}

//---------------------------------------------------
unsigned int PointCloudComp::getModelTextureID()
{
	return m_texModelID;
}

//---------------------------------------------------
unsigned int PointCloudComp::getNormalTextureID()
{
	return m_texNormalID;
}

//---------------------------------------------------
void PointCloudComp::registerKinectData(float planeDist, float pixelSize)
{
	m_planeDist = planeDist;
	m_pixelSize = pixelSize;
}