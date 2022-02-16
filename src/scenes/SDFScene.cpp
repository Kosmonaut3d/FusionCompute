#include "SDFScene.h"

#include "helper/fullScreenQuadRender.h"

//---------------------------------------------------
SDFScene::SDFScene()
    : m_sdfCompute{glm::vec3(-2, -2, -4), GUIScene::s_sdfResolution, 4}
    , m_sdfCPU{GUIScene::s_sdfResolution, glm::vec3(-2, -2, -4), 4, 2}
    , m_slice(glm::vec3(-1, 0, -2), 4)
{
}

//----------------------------------------------------------------------------------------------------------
void SDFScene::setup(ofxKinect& kinect)
{
}

//----------------------------------------------------------------------------------------------------------
void SDFScene::update(bool kinectUpdate, ofxKinect& kinect)
{
	m_sdfCompute.compute(0, 0);
}

//----------------------------------------------------------------------------------------------------------
void SDFScene::draw(ofCamera& camera)
{
	m_sdfCompute.drawOutline();
	//m_sdfCPU.drawRaymarch(camera, m_sdfCompute.getTextureID());
	m_slice.draw(ofMatrix4x4(m_sdfCompute.getWorldInv()), m_sdfCompute.getTextureID(), 0);
}
