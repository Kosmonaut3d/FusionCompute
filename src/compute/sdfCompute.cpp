#include "sdfCompute.h"
#include "scenes/GUIScene.h"

//----------------------------------------------------------------------------------------------------------
SDFCompute::SDFCompute(glm::vec3 origin, int resolution, float scale)
    : m_computeSDFShader{}
    , m_raymarchSDFShader{}
    , m_modelMat{}
    , m_modelMatInv{}
    , m_texID{}
    , m_resolution{resolution}
    , m_origin{origin}
    , m_scale{scale}
{
	if (!m_raymarchSDFShader.load("resources/vertShader.vert", "resources/raymarchSDF.frag"))
	{
		throw std::exception(); //"could not load shaders");
	}

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
	//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	std::vector<float> framedata(m_resolution * m_resolution * m_resolution * 2);
	std::fill(framedata.begin(), framedata.end(), m_scale/m_resolution);

	//glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, m_resolution, m_resolution, m_resolution, 0, GL_RED, GL_FLOAT, framedata.data());
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RG32F, m_resolution, m_resolution, m_resolution, 0, GL_RG, GL_FLOAT, framedata.data());

}

//----------------------------------------------------------------------------------------------------------
void SDFCompute::compute(unsigned int pointCloudId, unsigned int pointCloudNormalId, glm::mat4x4& viewToWorld, glm::mat4x4 worldToClipKinect)
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

	// Make the truncation distance dependent on the minimum distance between 2 tiles
	const float truncationScaled = m_scale / m_resolution * GUIScene::s_sdfTruncation;

	m_computeSDFShader.begin();

	m_computeSDFShader.setUniform1f("_stepSize", 1. / m_resolution);
	m_computeSDFShader.setUniformMatrix4f("_pclWorldToClip", worldToClipKinect);
	m_computeSDFShader.setUniformMatrix4f("_modelMatrix", m_modelMat);
	m_computeSDFShader.setUniform1f("_truncationDistance", truncationScaled);
	m_computeSDFShader.setUniform1f("_maxWeight", GUIScene::s_sdfWeightTruncation);

	glBindImageTexture(0, pointCloudId, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	glBindImageTexture(1, pointCloudNormalId, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	glBindImageTexture(2, m_texID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RG32F);

	m_computeSDFShader.dispatchCompute(m_resolution, m_resolution, m_resolution);
	m_computeSDFShader.end();
	/*
	std::vector<glm::vec2> framedata(m_resolution * m_resolution * m_resolution);
	glBindTexture(GL_TEXTURE_3D, m_texID);
	glGetTextureImage(m_texID, 0, GL_RED, GL_FLOAT, framedata.size() * sizeof(glm::vec2), &framedata[0]);
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

	ofSetColor(200, 0, 0);
	ofDrawBox(glm::vec3(.5, .5, .5), 1);

	ofPopMatrix();
	ofPopStyle();
}

//----------------------------------------------------------------------------------------------------------
void SDFCompute::drawRaymarch(ofCamera& camera)
{
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	ofPushStyle();
	m_raymarchSDFShader.begin();
	// glPolygonMode(GL_BACK, GL_LINE);
	float scalehalf = m_scale / 2;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, m_texID);

	// Make the truncation distance dependent on the minimum distance between 2 tiles
	const float truncationScaled = m_scale / m_resolution * GUIScene::s_sdfTruncation;

	m_raymarchSDFShader.setUniform3f("cameraWorld", camera.getPosition());
	m_raymarchSDFShader.setUniformMatrix4f("sdfBaseTransform", m_modelMatInv);
	m_raymarchSDFShader.setUniform1f("sdfResolution", m_resolution);
	m_raymarchSDFShader.setUniform1f("_truncationDistance", truncationScaled);
	m_raymarchSDFShader.setUniformMatrix4f("viewprojection", camera.getModelViewProjectionMatrix());
	m_raymarchSDFShader.setUniform1f("near", camera.getNearClip());
	m_raymarchSDFShader.setUniform1f("far", camera.getFarClip());

	ofDrawBox(m_origin + glm::vec3(scalehalf, scalehalf, scalehalf), m_scale, m_scale, m_scale);
	m_raymarchSDFShader.end();
	ofPopStyle();

	glDisable(GL_CULL_FACE);
}