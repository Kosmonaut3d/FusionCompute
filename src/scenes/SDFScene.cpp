#include "SDFScene.h"

#include "helper/fullScreenQuadRender.h"

//---------------------------------------------------
SDFScene::SDFScene()
    : m_sdfCompute{glm::vec3(-2, -2, -4), GUIScene::s_sdfResolution, 4}
    , m_sdfCPU{GUIScene::s_sdfResolution, glm::vec3(-2, -2, -4), 4, 2}
    , m_slice(glm::vec3(0, 0, -2), 4)
{
}

//----------------------------------------------------------------------------------------------------------
void SDFScene::setup(ofxKinect& kinect)
{
}

//----------------------------------------------------------------------------------------------------------
void SDFScene::update(bool kinectUpdate, ofxKinect& kinect)
{
	if (GUIScene::s_sdfCompute)
	{
		m_sdfCompute.compute(0, 0);
	}
}

//----------------------------------------------------------------------------------------------------------
void SDFScene::draw(ofCamera& camera)
{
	m_sdfCompute.drawOutline();
	if (GUIScene::s_sdfDrawRaytrace)
	{
		m_sdfCPU.drawRaymarch(camera, m_sdfCompute.getTextureID());
	}
	if (GUIScene::s_sdfDrawSlice)
	{
		m_slice.draw(ofMatrix4x4(m_sdfCompute.getWorldInv()), m_sdfCompute.getTextureID(), 0);
	}
}
