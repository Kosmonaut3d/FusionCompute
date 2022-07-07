#include "icpCompute.h"
#include "helper/eigen2glm.h"
#include "scenes/GUIScene.h"

//----------------------------------------------------------------------------------------------------------
ICPCompute::ICPCompute()
    : m_computeICPShader{}
    , m_computeICPSDFShader{}
    , m_computeICPReduction{}
    , m_correspondenceVisualizationTexID{}
    , m_atomicCounterID{}
    , m_ssboOutID{}
    , m_ssboCorrespondencesID{}
{
	m_computeICPShader.setupShaderFromFile(GL_COMPUTE_SHADER, "resources/computeICP.comp");
	m_computeICPShader.linkProgram();

	m_computeICPSDFShader.setupShaderFromFile(GL_COMPUTE_SHADER, "resources/computeICPSDF.comp");
	m_computeICPSDFShader.linkProgram();

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
	glGenTextures(1, &m_correspondenceVisualizationTexID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_correspondenceVisualizationTexID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w, tex_h, 0, GL_RGBA, GL_FLOAT, NULL);
	glBindImageTexture(1, m_correspondenceVisualizationTexID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	// atomic counter
	glGenBuffers(1, &m_atomicCounterID);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_atomicCounterID);
	glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), NULL, GL_STATIC_COPY);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

	// Set up ssbo
	glGenBuffers(1, &m_ssboOutID);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboOutID);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 1200 * sizeof(glm::vec4) * (12 + 2), nullptr, GL_DYNAMIC_READ);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind

	glGenBuffers(1, &m_ssboCorrespondencesID);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboCorrespondencesID);
	glBufferData(GL_SHADER_STORAGE_BUFFER, size * sizeof(ssbo_correspondence_data), nullptr, GL_DYNAMIC_COPY);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind
}

//----------------------------------------------------------------------------------------------------------
glm::mat4x4 ICPCompute::compute(unsigned int newVertexWorldTex, unsigned int newNormalWorldTex,
                                unsigned int oldVertexWorldTex, unsigned int oldNormalWorldTex,
                                glm::mat4x4& viewToWorldIt, glm::mat4x4& projection, SDFCompute& sdfCompute)
{
	glm::mat4x4                                   viewToWorldOld    = viewToWorldIt;
	glm::mat3x3                                   viewToWorldOldRot = glm::mat3x3(viewToWorldOld);
	glm::mat<4, 4, double, glm::precision::highp> viewToWorld_iter  = viewToWorldIt;

	for (int i = 0; i < GUIScene::s_ICP_GPU_iterations; i++)
	{
		glm::mat3x3 viewToWorldRot_iter = glm::mat3x3(viewToWorld_iter);

		// Clear buffer first
		constexpr glm::vec4 empty{0, 0, 0, 0};
		glClearTexImage(m_correspondenceVisualizationTexID, 0, GL_RGBA, GL_FLOAT, &empty);

		if (GUIScene::s_ICP_GPU_SDF)
		{
			// Camera
			glm::vec3 _cameraOrigin = viewToWorldIt * glm::vec4(0, 0, 0, 1);

			m_computeICPSDFShader.begin();

			m_computeICPSDFShader.setUniform3f("_cameraOrigin", _cameraOrigin);
			m_computeICPSDFShader.setUniformMatrix4f("_viewToWorldIt", viewToWorld_iter);
			m_computeICPSDFShader.setUniformMatrix3f("_viewToWorldItRot", viewToWorldRot_iter);
			m_computeICPSDFShader.setUniformMatrix4f("_sdfBaseTransform", sdfCompute.getSDFBaseTransformation());
			m_computeICPSDFShader.setUniform1f("_truncationDistance", sdfCompute.getScaledTruncation());
			m_computeICPSDFShader.setUniform1f("_epsilonDistance", GUIScene::s_ICP_epsilonDist);
			m_computeICPSDFShader.setUniform1f("_epsilonNormal", GUIScene::s_ICP_epsilonNor);

			glBindImageTexture(0, newVertexWorldTex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
			glBindImageTexture(1, newNormalWorldTex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
			glBindImageTexture(2, m_correspondenceVisualizationTexID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_3D, sdfCompute.getTextureID());

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboCorrespondencesID);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_ssboCorrespondencesID);

			// Reset counter
			unsigned int a = 0;
			glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_atomicCounterID);
			glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &a);
			glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, m_atomicCounterID);

			m_computeICPSDFShader.dispatchCompute(640, 480, 1);
			m_computeICPSDFShader.end();
		}
		else
		{
			computePointToPoint(viewToWorld_iter, projection, viewToWorldOld, viewToWorldRot_iter, viewToWorldOldRot,
			                    oldVertexWorldTex, newVertexWorldTex, oldNormalWorldTex, newNormalWorldTex);
		}

		GLuint correspondencesFound;
		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_atomicCounterID);
		glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &correspondencesFound);
		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
		GUIScene::s_ICP_GPU_correspondenceCount = correspondencesFound;

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		if (correspondencesFound <= 0)
		{
			break;
		}

		//////////////////////////////////////////
		// REDUCTION
		GLuint   query;
		GLuint64 elapsed_time;

		// Timing
		{
			glGenQueries(1, &query);
			glBeginQuery(GL_TIME_ELAPSED, query);
		}

		m_computeICPReduction.begin();
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboOutID);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_ssboOutID);

		const int dataSize = correspondencesFound;

		const int workgroupsize = 128;
		const int numworkgroups = dataSize / workgroupsize / 2;

		m_computeICPReduction.dispatchCompute(numworkgroups, 1, 1);
		m_computeICPReduction.end();

		glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(ssbo_out_data) * numworkgroups, &m_outData);

		// Timing
		{
			glEndQuery(GL_TIME_ELAPSED);
			glGetQueryObjectui64v(query, GL_QUERY_RESULT, &GUIScene::s_measureGPUTime_reduction);
		}

		// Feed the matrix
		calculateICP(viewToWorld_iter, numworkgroups);

		int test = 0;
	}

	return glm::mat4x4(viewToWorld_iter);
}

void ICPCompute::computePointToPoint(glm::highp_dmat4& viewToWorld_iter, glm::mat4x4& projection,
                                     glm::mat4x4& viewToWorld_old, glm::mat3x3& viewToWorldRot_iter,
                                     glm::mat3x3& viewToWorldRot_old, unsigned int oldVertexWorldTex,
                                     unsigned int newVertexWorldTex, unsigned int oldNormalWorldTex,
                                     unsigned int newNormalWorldTex)
{

	// Find correspondences
	m_computeICPShader.begin();

	m_computeICPShader.setUniformMatrix4f("viewToWorldIt", viewToWorld_iter);
	glm::mat4x4 viewProjection_iter = (projection * glm::inverse(glm::mat4x4(viewToWorld_iter)));
	m_computeICPShader.setUniformMatrix4f("viewProjectionIt", viewProjection_iter);

	m_computeICPShader.setUniformMatrix4f("viewToWorldOld", viewToWorld_old); // TODO, in first step this is correct
	m_computeICPShader.setUniformMatrix3f("viewToWorldItRot", viewToWorldRot_iter);
	m_computeICPShader.setUniformMatrix3f("viewToWorldOldRot", viewToWorldRot_old);

	m_computeICPShader.setUniform1f("_epsilonDistance", GUIScene::s_ICP_epsilonDist);
	m_computeICPShader.setUniform1f("_epsilonNormal", GUIScene::s_ICP_epsilonNor);

	// glBindImageTexture(0, depthTexID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
	glBindImageTexture(0, oldVertexWorldTex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	glBindImageTexture(1, newVertexWorldTex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	glBindImageTexture(2, oldNormalWorldTex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	glBindImageTexture(3, newNormalWorldTex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboCorrespondencesID);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_ssboCorrespondencesID);

	// correspondance
	glBindImageTexture(4, m_correspondenceVisualizationTexID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	// Reset counter
	unsigned int a = 0;
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_atomicCounterID);
	glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &a);
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, m_atomicCounterID);

	// 640*480 = 10*640 * 30 * 16
	m_computeICPShader.dispatchCompute(10, 30, 1);
	m_computeICPShader.end();
}

unsigned int ICPCompute::getTexID()
{
	return m_correspondenceVisualizationTexID;
}

void ICPCompute::feedVec3ToMatrix(Eigen::Matrix<double, 6, 6>& mat, const glm::vec4& v)
{
	mat << v.x, v.y, v.z;
}

void ICPCompute::calculateICP(glm::mat<4, 4, double, glm::precision::highp>& viewToWorld_iter, int numworkgroups)
{
	Eigen::Matrix<double, 6, 1> b_ = Eigen::Matrix<double, 6, 1>::Zero();
	Eigen::Matrix<double, 6, 6> A_ = Eigen::Matrix<double, 6, 6>::Zero();

	int corres = GUIScene::s_ICP_GPU_correspondenceCount / 128 / 2;

	for (size_t i = 0; i < numworkgroups; ++i)
	{
		auto                        obj = m_outData[i];
		Eigen::Matrix<double, 6, 6> A_i;
		feedOutputToMatrix(A_i, obj);

		if (isnan(A_i(0)))
		{
			continue;
		}
		A_ += A_i;

		Eigen::Matrix<double, 6, 1> b_i;
		b_i << obj.out_b0.x, obj.out_b0.y, obj.out_b0.z, obj.out_b1.x, obj.out_b1.y, obj.out_b1.z;

		b_ += b_i;
	}

	Eigen::Matrix<double, 6, 1> result = A_.bdcSvd(Eigen::ComputeFullU | Eigen::ComputeFullV).solve(b_);
	// Eigen::Matrix<double, 6, 1> result2 = A_.llt().solve(b_);
	// Eigen::Matrix<double, 6, 1> result3 = A_.jacobiSvd(Eigen::ComputeFullU | Eigen::ComputeFullV).solve(b_);

	if (!isnan(result[0]))
	{
		int test = 1;

		float b  = result[0];
		float g  = result[1];
		float a  = result[2];
		float tx = result[3];
		float ty = result[4];
		float tz = result[5];

		/*
		Eigen::Matrix<double, 4, 4> T_inc{{1, a, -g, tx}, {-a, 1, b, ty}, {g, -b, 1, tz}, {0, 0, 0, 1}};

		double alpha = result[0];
		double beta  = result[1];
		double gamma = result[2];

		Eigen::Matrix4d transformation{
		    {1, -gamma, beta, tx}, {gamma, 1, -alpha, ty}, {-beta, alpha, 1, tz}, {0, 0, 0, 1}};

		std::cout << transformation;

		*/
		// YET ANOTHER
		Eigen::Matrix4d transformation;
		double          alpha = result[0];
		double          beta  = result[1];
		double          gamma = result[2];

		transformation << 1, alpha * beta - gamma, alpha * gamma + beta, tx, gamma, alpha * beta * gamma + 1,
		    beta * gamma - alpha, ty, -beta, alpha, 1, tz, 0, 0, 0, 1;

		Eigen::Matrix<double, 4, 4> T_z = GLM2E(viewToWorld_iter);
		T_z                             = transformation * T_z;

		// Increment
		viewToWorld_iter = E2GLM(T_z);
	}
}

void ICPCompute::feedOutputToMatrix(Eigen::Matrix<double, 6, 6>& A_i, ICPCompute::ssbo_out_data& obj)
{
	A_i << obj.out_a00.x, obj.out_a00.y, obj.out_a00.z, obj.out_a01.x, obj.out_a01.y, obj.out_a01.z, obj.out_a10.x,
	    obj.out_a10.y, obj.out_a10.z, obj.out_a11.x, obj.out_a11.y, obj.out_a11.z, obj.out_a20.x, obj.out_a20.y,
	    obj.out_a20.z, obj.out_a21.x, obj.out_a21.y, obj.out_a21.z, obj.out_a30.x, obj.out_a30.y, obj.out_a30.z,
	    obj.out_a31.x, obj.out_a31.y, obj.out_a31.z, obj.out_a40.x, obj.out_a40.y, obj.out_a40.z, obj.out_a41.x,
	    obj.out_a41.y, obj.out_a41.z, obj.out_a50.x, obj.out_a50.y, obj.out_a50.z, obj.out_a51.x, obj.out_a51.y,
	    obj.out_a51.z;
}