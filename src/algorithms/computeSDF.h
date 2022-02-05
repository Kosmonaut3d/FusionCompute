#pragma once

#include "ofMain.h"

class computeSDF
{
public:
	computeSDF();
	void compute(ofTexture & depthImage);
	void draw(ofTexture& depthImage);
	unsigned int getTextureID();
	void registerKinectData(float planeDist, float pixelSize);

private:
	void setUpOutputTexture();

private:
	ofShader m_computeShader;
	ofBufferObject inPointBuffer;
	unsigned int m_texID;
	ofShader m_fsShader;
	float m_planeDist;
	float m_pixelSize;
};

 