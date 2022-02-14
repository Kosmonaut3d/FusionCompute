#pragma once

#include "ofMain.h"

class SDFCompute
{
  public:
	SDFCompute();
	void         compute(unsigned int pointCloudId, unsigned int pointCloudNormalId);
	unsigned int getTextureID();

  private:
	ofShader       m_computeSDFShader;
	unsigned int   m_texID;
};
