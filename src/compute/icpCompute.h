#pragma once

//#include "helper/eigen2glm.h"
#include "sdfCompute.h"
#include <../deps/Eigen/Dense>
#include <ofMain.h>

class ICPCompute
{
  public:
	ICPCompute();
	void        setupTexture();
	glm::mat4x4 ICPCompute::compute(unsigned int newVertexWorldTex, unsigned int newNormalWorldTex,
	                                unsigned int oldVertexWorldTex, unsigned int oldNormalWorldTex,
	                                glm::mat4x4& viewWorldIt, glm::mat4x4& projection, SDFCompute& sdfCompute);
	void computePointToPoint(glm::highp_dmat4& viewToWorld_iter, glm::mat4x4& projection, glm::mat4x4& viewToWorld_old,
	                         glm::mat3x3& viewToWorldRot_iter, glm::mat3x3& viewToWorldRot_old,
	                         unsigned int oldVertexWorldTex, unsigned int newVertexWorldTex,
	                         unsigned int oldNormalWorldTex, unsigned int newNormalWorldTex);
	unsigned int getTexID();
	void         calculateICP(glm::mat<4, 4, double, glm::precision::highp>& viewWorldIt, int numworkgroups);

  private:
	struct ssbo_correspondence_data
	{
		glm::vec4 o_a1; // PADDING
		glm::vec3 o_a2;
		float     o_b;
	};

	struct ssbo_out_data
	{
		glm::vec4 out_a00;
		glm::vec4 out_a01;
		glm::vec4 out_a10;
		glm::vec4 out_a11;
		glm::vec4 out_a20;
		glm::vec4 out_a21; // 3rd row, 4-6 column

		glm::vec4 out_a30;
		glm::vec4 out_a31;
		glm::vec4 out_a40;
		glm::vec4 out_a41;
		glm::vec4 out_a50;
		glm::vec4 out_a51; // 3rd row, 4-6 column

		glm::vec4 out_b0;
		glm::vec4 out_b1;
	};

	void feedOutputToMatrix(Eigen::Matrix<double, 6, 6>& A_i, ICPCompute::ssbo_out_data& obj);
	void feedVec3ToMatrix(Eigen::Matrix<double, 6, 6>& mat, const glm::vec4& v);

	ofShader     m_computeICPShader;
	ofShader     m_computeICPSDFShader;
	ofShader     m_computeICPReduction;
	unsigned int m_correspondenceVisualizationTexID;
	unsigned int m_atomicCounterID;
	unsigned int m_ssboOutID;
	unsigned int m_ssboCorrespondencesID;

	ssbo_out_data m_outData[1200];
};
