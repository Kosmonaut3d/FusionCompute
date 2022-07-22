#pragma once

#include "GUIScene.h"
#include "PointCloudScene.h"
#include "compute/icpCompute.h"
#include "compute/sdfCompute.h"
#include "cpuReference/sdf.h"
#include "cpuReference/slice.h"
#include "ofMain.h"
#include "ofxKinect.h"

class SDFScene
{
  public:
	SDFScene();
	void setup(ofxKinect& kinect);
	void update(bool kinectUpdate, ofxKinect& kinect, glm::mat4x4& viewToWorld, glm::mat4x4& worldToView,
	            glm::mat4x4& projection, unsigned int m_pointsCloudWorldTexNew, unsigned int m_pointsCloudNormalTexNew,
	            unsigned int m_pointsCloudWorldTexOld, unsigned int m_pointsCloudNormalTexOld);
	void draw(ofCamera& camera);

  private:
	SDFCompute          m_sdfCompute;
	SignedDistanceField m_sdfCPU;
	Slice               m_slice;
	ICPCompute          m_icpCompute;
	ofTexture*          m_kinectColorTexPtr;
};
