#pragma once

#include "ofMain.h"

class PointCloudComp
{
  public:
	PointCloudComp();
	void         compute(unsigned int depthTexID, bool isFrame0);
	unsigned int getModelTextureID(bool isFrame0);
	unsigned int getNormalTextureID(bool isFrame0);
	void         registerKinectData(float planeDist, float pixelSize);

  private:
	void setUpOutputTexture();

  private:
	ofShader       m_computeModelShader;
	ofShader       m_computeNormalShader;
	ofBufferObject inPointBuffer;
	unsigned int   m_texModelID_0;
	unsigned int   m_texModelID_1;
	unsigned int   m_texNormalID_0;
	unsigned int   m_texNormalID_1;
	float          m_planeDist;
	float          m_pixelSize;
};
