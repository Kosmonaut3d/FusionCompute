#pragma once

#include "ofMain.h"

class PointCloudComp
{
public:
	PointCloudComp();
	void compute(ofTexture & depthImage);
	unsigned int getModelTextureID();
	unsigned int getNormalTextureID();
	void registerKinectData(float planeDist, float pixelSize);

private:
	void setUpOutputTexture();

private:
	ofShader m_computeModelShader;
	ofShader m_computeNormalShader;
	ofBufferObject inPointBuffer;
	unsigned int m_texModelID;
	unsigned int m_texNormalID;
	float m_planeDist;
	float m_pixelSize;
};

 