#pragma once

#include "GUIScene.h"
#include "compute/BilateralBlurCompute.h"
#include "compute/PointCloudCompute.h"
#include "compute/PointCloudVis.h"
#include "cpuReference/IterativeClosestPointCPU.h"
#include "cpuReference/PointCloudCPU.h"
#include "ofMain.h"
#include "ofxKinect.h"

class PointCloudScene //: public SceneImpl
{
  public:
	PointCloudScene();
	void setup(ofxKinect& kinect, glm::mat4x4 viewToWorld);
	void update(bool kinectUpdate, ofxKinect& kinect, glm::mat4x4& viewToWorld, glm::mat4x4& worldToView,
	            glm::mat4x4& projection, bool isFrame0);
	void draw(ofCamera& camera, glm::mat4x4& viewToWorld, glm::mat4x4& worldToView, glm::mat4x4& projection,
	          bool isFrame0);
	void drawCameraOrientation(glm::mat4x4& viewToWorld, glm::mat4x4& worldToView, glm::mat4x4& projection);

	unsigned int getPCLWorld(bool isFrame0);
	unsigned int getPCLNormal(bool isFrame0);

  private:
	BilateralBlurCompute      m_bilateralBlurComp;
	PointCloudComp            m_pointCloudComp;
	PointCloudVis             m_pointCloudVis;
	PointCloudCPU             m_pointCloudCPU_0;
	PointCloudCPU             m_pointCloudCPU_1;
	IterativeClostestPointCPU m_icpCPU;
	ofTexture                 m_texDepthRaw;
	ofTexture*                m_texColorPtr;
};
