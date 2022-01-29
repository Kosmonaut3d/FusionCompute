#pragma once

#include "ofMain.h"

class computeSDF
{
public:
	computeSDF();
	void compute();
	void draw();
	ofTexture& getTexture();
	unsigned int getTextureID();

private:
	void setUpOutputTexture();

private:
	ofTexture outputTexture;
	ofShader computeShader;
	ofBufferObject inPointBuffer;
	unsigned int m_texID;
	ofShader m_fsShader;
};

 