#pragma once

#include "GUIScene.h"
#include "compute/BilateralBlurCompute.h"
#include "ofMain.h"
#include "ofxKinect.h"

class PreprocessDepthScene
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
