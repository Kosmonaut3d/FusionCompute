#pragma once

#include "ofMain.h"

class SDFCompute
{
  public:
	SDFCompute(glm::vec3 origin, int resolution, float scale);
	void         setupTexture();
	void         compute(unsigned int pointCloudId, unsigned int pointCloudNormalId, glm::mat4x4& viewToWorld,
	                     glm::mat4x4 worldToClipKinect);
	unsigned int getTextureID();
	glm::mat4x4& getWorldInv();
	void         drawOutline();
	void drawRaymarch(ofCamera& camera);

  private:
	ofShader     m_computeSDFShader;
	ofShader     m_raymarchSDFShader;
	glm::mat4x4  m_modelMat;
	glm::mat4x4  m_modelMatInv;
	unsigned int m_texID;
	int          m_resolution;
	glm::vec3    m_origin;
	float        m_scale;
};
