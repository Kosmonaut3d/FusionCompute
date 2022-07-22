#pragma once

#include "GUIScene.h"
#include "compute/BilateralBlurCompute.h"
#include "ofMain.h"
#include "ofxKinect.h"

/// <summary>
/// This scene can render the depth map and apply bilateral blur to it.
/// </summary>
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
