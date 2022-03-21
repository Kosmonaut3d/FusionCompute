#pragma once

#include "ofMain.h"

class ICPCompute
{
  public:
	ICPCompute(glm::vec3 origin, int resolution, float scale);
	void         setupTexture();
	void         ICPCompute::compute(unsigned int newVertexWorldTex);
	unsigned int getTexID();

  private:
	ofShader     m_computeICPShader;
	unsigned int m_texID;
};
