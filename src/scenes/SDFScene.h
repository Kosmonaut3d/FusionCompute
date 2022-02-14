#pragma once

#include "GUIScene.h"
#include "SceneImpl.h"
#include "ofMain.h"
#include "ofxKinect.h"
#include "compute/sdfCompute.h"

class SDFScene //: public SceneImpl
{
  public:
	SDFScene();
	void setup(ofxKinect& kinect);
	void update(bool kinectUpdate, ofxKinect& kinect);
	void draw();

  private:
	SDFCompute m_sdfCompute;
};
