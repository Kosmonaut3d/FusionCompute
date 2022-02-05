#pragma once

#include "ofMain.h"
#include "SceneImpl.h"
#include "ofxKinect.h"
#include "compute/PointCloudCompute.h"
#include "compute/PointCloudVis.h"
#include "GUIScene.h"

class PointCloudScene //: public SceneImpl
{
public:
	PointCloudScene();
	void setup(ofxKinect& kinect);
	void update(bool kinectUpdate, ofxKinect& kinect);
	void draw(ofCamera& camera);
	void drawOutline();

private:
	PointCloudComp m_pointCloudComp;
	PointCloudVis m_pointCloudVis;
	ofTexture m_texDepthRaw;
	ofTexture* m_texColorPtr;
};
