#include "icpCompute.h"
#include "scenes/GUIScene.h"

//----------------------------------------------------------------------------------------------------------
ICPCompute::ICPCompute()
    : m_computeICPShader{}
    , m_computeICPReduction{}
    , m_texID{}
    , m_atomicCounterID{}
    , m_ssboInID{}
    , m_ssboOutID{}
    , m_ssboCorrespondencesID{}
{
	m_computeICPShader.setupShaderFromFile(GL_COMPUTE_SHADER, "resources/computeICP.comp");
	m_computeICPShader.linkProgram();

	m_computeICPReduction.setupShaderFromFile(GL_COMPUTE_SHADER, "resources/ICPReduction.comp");
	m_computeICPReduction.linkProgram();

	setupTexture();
}

void ICPCompute::setupTexture()
{
	// dimensions of the image
	const int tex_w = 640, tex_h = 480;
	const int size = tex_w * tex_h;

	// Model
	glGenTextures(1, &m_texID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_texID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w, tex_h, 0, GL_RGBA, GL_FLOAT, NULL);
	glBindImageTexture(1, m_texID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	// atomic counter
	glGenBuffers(1, &m_atomicCounterID);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_atomicCounterID);
	glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), NULL, GL_STATIC_COPY);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

	// Set up ssbo
	glGenBuffers(1, &m_ssboInID);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboInID);
	glBufferData(GL_SHADER_STORAGE_BUFFER, size * sizeof(unsigned int), nullptr, GL_DYNAMIC_READ);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind

	glGenBuffers(1, &m_ssboOutID);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboOutID);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 300 * sizeof(unsigned int), nullptr, GL_DYNAMIC_READ);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind

	glGenBuffers(1, &m_ssboCorrespondencesID);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboCorrespondencesID);
	glBufferData(GL_SHADER_STORAGE_BUFFER, size * sizeof(ssbo_correspondence_data), nullptr, GL_DYNAMIC_READ);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind
}

//----------------------------------------------------------------------------------------------------------
void ICPCompute::compute(unsigned int newVertexWorldTex, unsigned int newNormalWorldTex, unsigned int oldVertexWorldTex,
                         unsigned int oldNormalWorldTex, glm::mat4x4& viewWorldIt, glm::mat4x4& viewProjectionIt)
{
	glm::mat3x3 viewToWorldRot_prev = glm::mat3x3(viewWorldIt);

	// Clear buffer first
	constexpr glm::vec4 empty{0, 0, 0, 0};
	glClearTexImage(m_texID, 0, GL_RGBA, GL_FLOAT, &empty);

	// Find correspondences
	m_computeICPShader.begin();

	m_computeICPShader.setUniformMatrix4f("viewToWorldIt", viewWorldIt);
	m_computeICPShader.setUniformMatrix4f("viewToWorldOld", viewWorldIt); // TODO, in first step this is correct
	m_computeICPShader.setUniformMatrix3f("viewToWorldItRot", viewToWorldRot_prev); // TODO
	m_computeICPShader.setUniformMatrix4f("viewProjectionIt", viewProjectionIt);
	m_computeICPShader.setUniform1f("_epsilonDistance", GUIScene::s_ICP_epsilonDist);

	// glBindImageTexture(0, depthTexID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
	glBindImageTexture(0, oldVertexWorldTex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	glBindImageTexture(1, newVertexWorldTex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	glBindImageTexture(2, oldNormalWorldTex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	glBindImageTexture(3, newNormalWorldTex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboCorrespondencesID);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_ssboCorrespondencesID);

	// correspondance
	glBindImageTexture(4, m_texID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	// Reset counter
	unsigned int a = 0;
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_atomicCounterID);
	glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &a);
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, m_atomicCounterID);

	m_computeICPShader.dispatchCompute(640, 480, 1);
	m_computeICPShader.end();

	GLuint counter;
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_atomicCounterID);
	glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &counter);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
	GUIScene::s_ICPGPU_correspondences = counter;

	ssbo_correspondence_data data[20];
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(data), &data);

	/*
	std::vector<glm::vec4> framedata(640 * 480);
	glBindTexture(GL_TEXTURE_2D, m_texID);
	glGetTextureImage(m_texID, 0, GL_RGBA, GL_FLOAT, framedata.size() * sizeof(glm::vec4), &framedata[0]);
	*/

	// READ ATOMIC
	/*
	 */

	//////////////////////////////////////////
	// REDUCTION
	GLuint   query;
	GLuint64 elapsed_time;

	// Timing
	{
		glGenQueries(1, &query);
		glBeginQuery(GL_TIME_ELAPSED, query);
	}

	const int dataSize   = 640 * 480;
	auto      test2      = log2(dataSize);
	auto      ceiling    = ceil(test2);
	int       iterations = 1 << (static_cast<int>(ceil(test2)) - 1);

	m_computeICPReduction.begin();
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboInID);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_ssboInID);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboOutID);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_ssboOutID);

	m_computeICPReduction.setUniform1i("iterations", iterations);
	m_computeICPReduction.setUniform1i("datasize", dataSize);

	const int workgroupsize = 1024;
	const int numworkgroups = dataSize / workgroupsize / 2;

	m_computeICPReduction.dispatchCompute(numworkgroups, 1, 1);
	m_computeICPReduction.end();

	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(m_outData), &m_outData);

	GLuint combined = 0;

	for (int i = 0; i < 300; i++)
	{
		combined += m_outData[i];
	}

	// Timing
	{
		glEndQuery(GL_TIME_ELAPSED);
		glGetQueryObjectui64v(query, GL_QUERY_RESULT, &GUIScene::s_measureGPUTime_reduction);
	}

	int test = 0;
}

unsigned int ICPCompute::getTexID()
{
	return m_texID;
}
