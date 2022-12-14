#include "sdfCompute.h"
#include "scenes/GUIScene.h"

//----------------------------------------------------------------------------------------------------------
SDFCompute::SDFCompute(glm::vec3 origin, int resolution, float scale)
    : m_computeSDFShader{}
    , m_computeSDFColorShader{}
    , m_rayMarchSDFShader{}
    , m_rayMarchSDFColorShader{}
    , m_expandSDFShader{}
    , m_modelMat{}
    , m_modelMatInv{}
    , m_texID{}
    , m_colorTexID{}
    , m_atomicCounterID{}
    , m_resolution{resolution}
    , m_origin{origin}
    , m_scale{scale}
{
    if (!m_rayMarchSDFShader.load("shaders/vertShader.vert", "shaders/raymarchSDF.frag"))
	{
		throw std::exception(); //"could not load shaders");
	}

    if (!m_rayMarchSDFColorShader.load("shaders/vertShader.vert", "shaders/raymarchSDFColor.frag"))
	{
		throw std::exception(); //"could not load shaders");
	}

	m_computeSDFShader.setupShaderFromFile(GL_COMPUTE_SHADER, "shaders/computeSDF.comp");
	m_computeSDFShader.linkProgram();

	m_computeSDFColorShader.setupShaderFromFile(GL_COMPUTE_SHADER, "shaders/computeSDFColor.comp");
	m_computeSDFColorShader.linkProgram();

	m_modelMat    = glm::mat4x4();
	m_modelMat    = glm::scale(glm::translate(m_modelMat, origin), glm::vec3(scale));
	m_modelMatInv = glm::inverse(m_modelMat);
	m_resolution  = GUIScene::s_sdfResolution;
	setupTexture();
	setupColorTexture();
	setupAtomicCounter();
}

void SDFCompute::setupAtomicCounter()
{
	// atomic counter
	glGenBuffers(1, &m_atomicCounterID);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_atomicCounterID);
	glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), NULL, GL_STATIC_COPY);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
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
	GLuint query;
	if (GUIScene::s_measureTime)
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

	const int threadsX     = 4;
	const int threadsYZ    = 8;
	const int resolutionX  = max(m_resolution / threadsX, 1);
	const int resolutionYZ = max(m_resolution / threadsYZ, 1);

	currentShader->dispatchCompute(resolutionX, resolutionYZ, resolutionYZ);
	currentShader->end();

	if (GUIScene::s_measureTime)
	{
		glEndQuery(GL_TIME_ELAPSED);
		glGetQueryObjectui64v(query, GL_QUERY_RESULT, &GUIScene::s_sdfMeasuredComputeTime);
	}
}

// Experimental
void SDFCompute::computeExpandSDF()
{
	GLuint query;
	if (GUIScene::s_measureTime)
	{
		glGenQueries(1, &query);
		glBeginQuery(GL_TIME_ELAPSED, query);
	}

	m_expandSDFShader.begin();

	// bind counter
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_atomicCounterID);
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, m_atomicCounterID);
	unsigned int a = 0;
	glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &a);

	float distanceBetweenFields = 1. / m_resolution * m_scale;
	m_expandSDFShader.setUniform1f("_distance_x", distanceBetweenFields);
	m_expandSDFShader.setUniform1f("_truncationDistance", getScaledTruncation());
	m_expandSDFShader.setUniform1i("_resolution", m_resolution);

	glm::ivec3 testDir[4] = {glm::ivec3(0, 0, 1), glm::ivec3(-1, 0, 0), glm::ivec3(0, 0, 1), glm::ivec3(0, 0, -1)};
	static int i          = 0;
	i++;
	if (i >= 4)
	{
		i = 0;
	}

	m_expandSDFShader.setUniform3i("_testDirection", testDir[i].x, testDir[i].y, testDir[i].z);

	glBindImageTexture(0, m_texID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RG32F);

	const int threadsize         = 8;
	const int resolutionDispatch = max(m_resolution / threadsize, 1);
	m_expandSDFShader.dispatchCompute(resolutionDispatch, resolutionDispatch, resolutionDispatch);
	m_expandSDFShader.end();

	GLuint numOperations;
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_atomicCounterID);
	glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &numOperations);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

	if (GUIScene::s_measureTime)
	{
		glEndQuery(GL_TIME_ELAPSED);
		glGetQueryObjectui64v(query, GL_QUERY_RESULT, &GUIScene::s_sdfExpandMeasuredComputeTime);
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
	GLuint query;

	if (GUIScene::SceneSelection::SDF == GUIScene::s_sceneSelection)
	{
		glGenQueries(1, &query);
		glBeginQuery(GL_TIME_ELAPSED, query);
	}

	// glEnable(GL_CULL_FACE);
	// glCullFace(GL_BACK);

	ofPushStyle();

	bool drawColor = GUIScene::s_sdfComputeColor && !GUIScene::s_sdfDrawNormals;

	const ofShader* currentShader = drawColor ? &m_rayMarchSDFColorShader : &m_rayMarchSDFShader;

	currentShader->begin();
	// glPolygonMode(GL_BACK, GL_LINE);
	float scalehalf = m_scale / 2;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, m_texID);

	if (drawColor)
	{
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_3D, m_colorTexID);
	}
	else
	{
		currentShader->setUniform1i("_drawNormals", GUIScene::s_sdfDrawNormals ? 1 : 0);
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
