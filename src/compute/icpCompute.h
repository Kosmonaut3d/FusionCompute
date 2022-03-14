#pragma once

#include "ofMain.h"

class ICPCompute
{
  public:
	ICPCompute(glm::vec3 origin, int resolution, float scale);
	void setupTexture();
	void compute(unsigned int pointCloudId, unsigned int pointCloudNormalId, glm::mat4x4& viewToWorld,
	             glm::mat4x4 worldToClipKinect);

  private:
	ofShader     m_computeSDFShader;
	unsigned int m_texID;
};
