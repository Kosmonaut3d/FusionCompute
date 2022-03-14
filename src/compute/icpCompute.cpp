#include "icpCompute.h"
#include "scenes/GUIScene.h"

//----------------------------------------------------------------------------------------------------------
ICPCompute::ICPCompute(glm::vec3 origin, int resolution, float scale)
    : m_computeSDFShader{}
    , m_texID{}
{
	m_computeSDFShader.setupShaderFromFile(GL_COMPUTE_SHADER, "resources/computeSDF.comp");
	m_computeSDFShader.linkProgram();

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
void ICPCompute::compute(unsigned int pointCloudId, unsigned int pointCloudNormalId, glm::mat4x4& viewToWorld,
                         glm::mat4x4 worldToClipKinect)
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
	m_computeSDFShader.setUniformMatrix4f("_viewToWorld", viewToWorld);
	m_computeSDFShader.setUniformMatrix3f("_viewToWorldRot", glm::mat3(viewToWorld));
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

	if (GUIScene::SceneSelection::SDF == GUIScene::s_sceneSelection)
	{
		glEndQuery(GL_TIME_ELAPSED);
		glGetQueryObjectui64v(query, GL_QUERY_RESULT, &GUIScene::s_measureGPUTime);
	}
}

//----------------------------------------------------------------------------------------------------------
unsigned int ICPCompute::getTextureID()
{
	return m_texID;
}

//----------------------------------------------------------------------------------------------------------
glm::mat4x4& ICPCompute::getWorldInv()
{
	return m_modelMatInv;
}

//----------------------------------------------------------------------------------------------------------
void ICPCompute::drawOutline()
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
void ICPCompute::drawRaymarch(ofCamera& camera)
{
	GLuint   query;
	GLuint64 elapsed_time;

	if (GUIScene::SceneSelection::SDF == GUIScene::s_sceneSelection)
	{
		glGenQueries(1, &query);
		glBeginQuery(GL_TIME_ELAPSED, query);
	}

	// glEnable(GL_CULL_FACE);
	// glCullFace(GL_BACK);

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

	// glDisable(GL_CULL_FACE);

	if (GUIScene::SceneSelection::SDF == GUIScene::s_sceneSelection)
	{
		glEndQuery(GL_TIME_ELAPSED);
		glGetQueryObjectui64v(query, GL_QUERY_RESULT, &GUIScene::s_measureGPUTime2);
	}
}