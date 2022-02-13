#pragma once

#include "GUIScene.h"
#include "SceneImpl.h"
#include "ofMain.h"
#include "ofxKinect.h"
#include "compute/BilateralBlurCompute.h"

class PreprocessDepthScene //: public SceneImpl
{
  public:
	PreprocessDepthScene();
	void setup(ofxKinect& kinect);
	void update(bool kinectUpdate, ofxKinect& kinect);
	void draw();

  private:
	BilateralBlurCompute m_bilateralBlurComp;
	ofTexture            m_texDepthRaw;
};
