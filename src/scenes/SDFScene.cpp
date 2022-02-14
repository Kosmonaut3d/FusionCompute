#include "SDFScene.h"

#include "helper/fullScreenQuadRender.h"

//---------------------------------------------------
SDFScene::SDFScene()
    : m_sdfCompute{
          glm::vec3(-2, -2, -4),
          GUIScene::s_sdfResolution, 4
      }
{
	
}

//----------------------------------------------------------------------------------------------------------
void SDFScene::setup(ofxKinect& kinect)
{
}

//----------------------------------------------------------------------------------------------------------
void SDFScene::update(bool kinectUpdate, ofxKinect& kinect)
{
}

//----------------------------------------------------------------------------------------------------------
void SDFScene::draw()
{
	m_sdfCompute.drawOutline();
}

