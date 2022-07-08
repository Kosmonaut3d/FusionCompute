#include "sdfCompute.h"
#include "scenes/GUIScene.h"

//----------------------------------------------------------------------------------------------------------
SDFCompute::SDFCompute(glm::vec3 origin, int resolution, float scale)
    : m_computeSDFShader{}
    , m_computeSDFColorShader{}
    , m_raymarchSDFShader{}
    , m_raymarchSDFColorShader{}
    , m_modelMat{}
    , m_modelMatInv{}
    , m_texID{}
    , m_colorTexID{}
    , m_resolution{resolution}
    , m_origin{origin}
    , m_scale{scale}
{
	if (!m_raymarchSDFShader.load("resources/vertShader.vert", "resources/raymarchSDF.frag"))
	{
		throw std::exception(); //"could not load shaders");
	}

	if (!m_raymarchSDFColorShader.load("resources/vertShader.vert", "resources/raymarchSDFColor.frag"))
	{
		throw std::exception(); //"could not load shaders");
	}

	m_computeSDFShader.setupShaderFromFile(GL_COMPUTE_SHADER, "resources/computeSDF.comp");
	m_computeSDFShader.linkProgram();

	m_computeSDFColorShader.setupShaderFromFile(GL_COMPUTE_SHADER, "resources/computeSDFColor.comp");
	m_computeSDFColorShader.linkProgram();

	m_modelMat    = glm::mat4x4();
	m_modelMat    = glm::scale(glm::translate(m_modelMat, origin), glm::vec3(scale));
	m_modelMatInv = glm::inverse(m_modelMat);
	m_resolution  = GUIScene::s_sdfResolution;
	setupTexture();
	setupColorTexture();
}

void SDFCompute::setupTexture()
{
	glGenTextures(1, &m_texID);
	glBindTexture(GL_TEXTURE_3D, m_texID);
	// glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	std::vector<glm::vec2> framedata(m_resolution * m_resolution * m_resolution);

	const float truncationScaled = getScaledTruncation();
	std::fill(framedata.begin(), framedata.end(), glm::vec2(truncationScaled, 0));

	glTexImage3D(GL_TEXTURE_3D, 0, GL_RG32F, m_resolution, m_resolution, m_resolution, 0, GL_RG, GL_FLOAT,
	             framedata.data());

	glBindTexture(GL_TEXTURE_3D, 0);
}

void SDFCompute::setupColorTexture()
{
	glGenTextures(1, &m_colorTexID);
	glBindTexture(GL_TEXTURE_3D, m_colorTexID);
	// glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, m_resolution, m_resolution, m_resolution, 0, GL_RGBA, GL_FLOAT, nullptr);
	glBindTexture(GL_TEXTURE_3D, 0);
}

//----------------------------------------------------------------------------------------------------------
void SDFCompute::compute(unsigned int pointCloudId, unsigned int pointCloudNormalId, unsigned int kinectColorTexId,
                         glm::mat4x4& viewToWorld, glm::mat4x4 worldToClipKinect)
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

		// TODO Add a check, since this takes ages
		glDeleteTextures(1, &m_colorTexID);
		setupColorTexture();
	}

	const ofShader* currentShader = GUIScene::s_sdfComputeColor ? &m_computeSDFColorShader : &m_computeSDFShader;

	currentShader->begin();

	// Make the truncation distance dependent on the minimum distance between 2 tiles
	const float truncationScaled = getScaledTruncation();

	currentShader->setUniform1f("_stepSize", 1. / m_resolution);
	currentShader->setUniformMatrix4f("_pclWorldToClip", worldToClipKinect);
	currentShader->setUniformMatrix4f("_viewToWorld", viewToWorld);
	currentShader->setUniformMatrix3f("_viewToWorldRot", glm::mat3(viewToWorld));
	currentShader->setUniformMatrix4f("_modelMatrix", m_modelMat);
	currentShader->setUniform1f("_truncationDistance", truncationScaled);
	currentShader->setUniform1f("_maxWeight", GUIScene::s_sdfWeightTruncation);

	if (GUIScene::s_sdfComputeColor)
	{
		glm::vec3 _cameraOrigin = viewToWorld * glm::vec4(0, 0, 0, 1);

		currentShader->setUniform3f("_cameraOrigin", _cameraOrigin);

		int test = currentShader->getUniformLocation("color_input");
		glBindImageTexture(0, pointCloudId, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
		glBindImageTexture(1, pointCloudNormalId, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
		glBindImageTexture(2, m_texID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RG32F);
		glBindImageTexture(3, m_colorTexID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);
		// glBindImageTexture(2, kinectColorTexId, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG8UI);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, kinectColorTexId);
	}
	else
	{
		glBindImageTexture(0, pointCloudId, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
		glBindImageTexture(1, pointCloudNormalId, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
		glBindImageTexture(2, m_texID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RG32F);
	}

	const int threads    = 8;
	const int resolution = max(m_resolution / threads, 1);

	currentShader->dispatchCompute(resolution, resolution, resolution);
	currentShader->end();

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
glm::mat4x4& SDFCompute::getSDFBaseTransformation()
{
	return m_modelMatInv;
}

float SDFCompute::getScaledTruncation()
{
	return m_scale * GUIScene::s_sdfTruncation;
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

	const ofShader* currentShader = GUIScene::s_sdfComputeColor ? &m_raymarchSDFColorShader : &m_raymarchSDFShader;

	currentShader->begin();
	// glPolygonMode(GL_BACK, GL_LINE);
	float scalehalf = m_scale / 2;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, m_texID);

	if (GUIScene::s_sdfComputeColor)
	{
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_3D, m_colorTexID);
	}

	// Make the truncation distance dependent on the minimum distance between 2 tiles
	const float truncationScaled = getScaledTruncation();

	currentShader->setUniform3f("cameraWorld", camera.getPosition());
	currentShader->setUniformMatrix4f("sdfBaseTransform", m_modelMatInv);
	currentShader->setUniform1f("sdfResolution", m_resolution);
	currentShader->setUniform1f("_truncationDistance", truncationScaled);
	currentShader->setUniformMatrix4f("viewprojection", camera.getModelViewProjectionMatrix());

	ofDrawBox(m_origin + glm::vec3(scalehalf, scalehalf, scalehalf), m_scale, m_scale, m_scale);
	currentShader->end();
	ofPopStyle();

	// glDisable(GL_CULL_FACE);

	if (GUIScene::SceneSelection::SDF == GUIScene::s_sceneSelection)
	{
		glEndQuery(GL_TIME_ELAPSED);
		glGetQueryObjectui64v(query, GL_QUERY_RESULT, &GUIScene::s_measureGPUTime2);
	}
}