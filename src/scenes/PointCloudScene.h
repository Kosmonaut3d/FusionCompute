#pragma once

#include "GUIScene.h"
#include "SceneImpl.h"
#include "compute/PointCloudCompute.h"
#include "compute/PointCloudVis.h"
#include "cpuReference/PointCloudCPU.h"
#include "ofMain.h"
#include "ofxKinect.h"

class PointCloudScene //: public SceneImpl
{
  public:
	PointCloudScene();
	void setup(ofxKinect& kinect);
	void update(bool kinectUpdate, ofxKinect& kinect);
	void draw(ofCamera& camera);
	void drawOutline();
	void drawTest();

  private:
	PointCloudComp m_pointCloudComp;
	PointCloudVis  m_pointCloudVis;
	PointCloudCPU  m_pointCloudCPU;
	ofTexture      m_texDepthRaw;
	ofTexture*     m_texColorPtr;
};
