#pragma once

#include "ofMain.h"

/// <summary>
///	This class sets up and dispatches compute shaders that calculate bilateral blur for a given depth map
/// </summary>
class BilateralBlurCompute
{
  public:
	BilateralBlurCompute();
	void         compute(ofTexture& depthImage, bool blur);
	unsigned int getTextureID();

  private:
	ofShader     m_computeBlurShader;
	unsigned int m_texID;
};
