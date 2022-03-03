#pragma once

#include "GUIScene.h"
#include "PointCloudScene.h"
#include "SceneImpl.h"
#include "compute/sdfCompute.h"
#include "cpuReference/sdf.h"
#include "cpuReference/slice.h"
#include "ofMain.h"
#include "ofxKinect.h"

class SDFScene //: public SceneImpl
{
  public:
	SDFScene();
	void setup(ofxKinect& kinect);
	void update(bool kinectUpdate, ofxKinect& kinect, glm::mat4x4& worldToClip, glm::mat4x4& viewToWorld,
	            unsigned int m_pointsCloudWorldTex,
	            unsigned int m_pointsCloudNormalTex);
	void draw(ofCamera& camera);

  private:
	SDFCompute          m_sdfCompute;
	SignedDistanceField m_sdfCPU;
	Slice               m_slice;
};
