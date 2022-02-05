#pragma once
#include <ofMain.h>
#include <ofxKinect.h>

class PointCloudComp
{
public: 
	PointCloudComp();

	void draw(unsigned int pointCloudTexId, unsigned int rgbTexId, bool drawNormals, glm::mat4x4& mvpMat, float factor);


private:
	ofShader m_shader;
};

