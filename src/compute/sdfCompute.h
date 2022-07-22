#pragma once

#include "ofMain.h"

/// <summary>
///	This class sets up and dispatches compute shaders that calculate the projective TSDF integration and fusion.
/// </summary>
class SDFCompute
{
  public:
	SDFCompute(glm::vec3 origin, int resolution, float scale);
	void         setupAtomicCounter();
	void         setupTexture();
	void         setupColorTexture();
	void         compute(unsigned int pointCloudId, unsigned int pointCloudNormalId, unsigned int kinectColorTexId,
	                     glm::mat4x4& viewToWorld, glm::mat4x4 worldToClipKinect);
	void         computeExpandSDF();
	unsigned int getTextureID();
	glm::mat4x4& getSDFBaseTransformation();
	float        getScaledTruncation();
	void         drawOutline();
	void         drawRaymarch(ofCamera& camera);

  private:
	ofShader m_computeSDFShader;
	ofShader m_computeSDFColorShader;
	ofShader m_rayMarchSDFShader;
	ofShader m_rayMarchSDFColorShader;

	ofShader m_expandSDFShader;

	glm::mat4x4  m_modelMat;
	glm::mat4x4  m_modelMatInv;
	unsigned int m_texID;
	unsigned int m_colorTexID;
	unsigned int m_atomicCounterID;
	int          m_resolution;
	glm::vec3    m_origin;
	float        m_scale;
};
