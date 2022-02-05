#pragma once
#pragma once

#include "ofMain.h"

class FullScreenQuadRender
{
public:
	FullScreenQuadRender();
	void draw(ofTexture tex);
	void draw(unsigned int texID, int texTarget);

	static FullScreenQuadRender& get();

private:
	ofShader m_shader;
};
