#pragma once

#include "ofMain.h"

class PointCloudComp
{
public:
	PointCloudComp();
	void compute(ofTexture & depthImage);
	unsigned int getTextureID();
	void registerKinectData(float planeDist, float pixelSize);

private:
	void setUpOutputTexture();

private:
	ofShader m_computeShader;
	ofBufferObject inPointBuffer;
	unsigned int m_texID;
	float m_planeDist;
	float m_pixelSize;
};

 