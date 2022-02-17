#include "sdfCompute.h"
#include "scenes/GUIScene.h"

//----------------------------------------------------------------------------------------------------------
SDFCompute::SDFCompute(glm::vec3 origin, int resolution, float scale)
    : m_computeSDFShader{}
    , m_modelMat{}
    , m_modelMatInv{}
    , m_texID{}
    , m_resolution{resolution}
    , m_origin{origin}
    , m_scale{scale}
{
	m_computeSDFShader.setupShaderFromFile(GL_COMPUTE_SHADER, "resources/computeSDF.comp");
	m_computeSDFShader.linkProgram();

	m_modelMat = glm::mat4x4();
	m_modelMat = glm::scale(glm::translate(m_modelMat, origin), glm::vec3(scale));
	m_modelMatInv = glm::inverse(m_modelMat);
	setupTexture();
}

void SDFCompute::setupTexture()
{
	glGenTextures(1, &m_texID);
	glBindTexture(GL_TEXTURE_3D, m_texID);
	std::vector<float> framedata(m_resolution * m_resolution * m_resolution);
	std::fill(framedata.begin(), framedata.end(), 2.0f);
	//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, m_resolution, m_resolution, m_resolution, 0, GL_RED, GL_FLOAT, framedata.data());
	//glTexImage3D(GL_TEXTURE_3D, 0, GL_RG32F, m_resolution, m_resolution, m_resolution, 0, GL_RG, GL_FLOAT, NULL);

}

//----------------------------------------------------------------------------------------------------------
void SDFCompute::compute(unsigned int pointCloudId, unsigned int pointCloudNormalId)
{
	GLuint   query;
	GLuint64 elapsed_time;

	if (GUIScene::SceneSelection::SDF == GUIScene::s_sceneSelection)
	{
		glGenQueries(1, &query);
		glBeginQuery(GL_TIME_ELAPSED, query);
	}

	if (GUIScene::s_sdfResolution != m_resolution)
	{
		m_resolution = GUIScene::s_sdfResolution;
		glDeleteTextures(1, &m_texID);
		setupTexture();
	}

	glm::vec3 point = GUIScene::s_testPointPos;

	glm::vec3 pointLocal = m_modelMatInv * glm::vec4(point, 1);


	m_computeSDFShader.begin();

	m_computeSDFShader.setUniformMatrix4f("_modelMatrix", m_modelMat);
	m_computeSDFShader.setUniform3f("_point", point);
	m_computeSDFShader.setUniform1f("_stepSize", 1./m_resolution);
	glBindImageTexture(0, m_texID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);

	m_computeSDFShader.dispatchCompute(m_resolution, m_resolution, m_resolution);
	m_computeSDFShader.end();

	/* std::vector<float> framedata(m_resolution * m_resolution * m_resolution);
	glBindTexture(GL_TEXTURE_3D, m_texID);
	glGetTextureImage(m_texID, 0, GL_RED, GL_FLOAT, framedata.size()*sizeof(float), &framedata[0]);
	*/

	int test = 1;

	if (GUIScene::SceneSelection::SDF == GUIScene::s_sceneSelection)
	{
		glEndQuery(GL_TIME_ELAPSED);
		glGetQueryObjectui64v(query, GL_QUERY_RESULT, &GUIScene::s_measureGPUTime);
	}
}

//----------------------------------------------------------------------------------------------------------
unsigned int SDFCompute::getTextureID()
{
	return m_texID;
}

//----------------------------------------------------------------------------------------------------------
glm::mat4x4& SDFCompute::getWorldInv()
{
	return m_modelMatInv;
}

//----------------------------------------------------------------------------------------------------------
void SDFCompute::drawOutline()
{
	ofPushStyle();
	ofPushMatrix();

	ofGetCurrentRenderer()->setFillMode(ofFillFlag::OF_OUTLINE);
	ofSetColor(200, 100, 200);
	ofMultMatrix(m_modelMat);
	ofNoFill();
	ofDrawBox(0, 0, 0, .1f, .1f, .1f);
	ofDrawBox(1, 1, 1, .1f, .1f, .1f);

	ofPopMatrix();
	ofPopStyle();

	//ofGetCurrentRenderer()->setFillMode(ofFillFlag::OF_OUTLINE);
	//ofSetColor(10, 100, 200);
	//float scalehalf = m_scale / 2;
	//ofDrawBox(m_origin + glm::vec3(scalehalf, scalehalf, scalehalf), m_scale, m_scale, m_scale);
}
