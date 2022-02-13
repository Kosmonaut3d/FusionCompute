#pragma once

#include "ofMain.h"

class BilateralBlurCompute
{
  public:
	BilateralBlurCompute();
	void         compute(ofTexture& depthImage, bool blur);
	unsigned int getTextureID();

  private:
	ofShader       m_computeBlurShader;
	unsigned int   m_texID;
};
