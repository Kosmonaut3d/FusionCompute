#pragma once

#include "ofMain.h"

class ICPCompute
{
  public:
	ICPCompute();
	void setupTexture();
	void ICPCompute::compute(unsigned int newVertexWorldTex, unsigned int oldVertexWorldTex, glm::mat4x4& viewWorldIt,
	                         glm::mat4x4& viewProjectionIt);
	unsigned int getTexID();

  private:
	ofShader     m_computeICPShader;
	ofShader     m_computeICPReduction;
	unsigned int m_texID;
	unsigned int m_atomicCounterID;
	unsigned int m_ssboID;
};
