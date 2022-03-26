#include "SDFScene.h"

#include "helper/fullScreenQuadRender.h"

//---------------------------------------------------
SDFScene::SDFScene()
    : m_sdfCompute{glm::vec3(-1, -1, -2), GUIScene::s_sdfResolution, 2}
    , m_sdfCPU{GUIScene::s_sdfResolution, glm::vec3(-2, -2, -4), 4, 2}
    , m_slice(glm::vec3(0, 0, -1), 2)
    , m_icpCompute{}
{
}

//----------------------------------------------------------------------------------------------------------
void SDFScene::setup(ofxKinect& kinect)
{
}

//----------------------------------------------------------------------------------------------------------
void SDFScene::update(bool kinectUpdate, ofxKinect& kinect, glm::mat4x4& worldToClip, glm::mat4x4& viewToWorld,
                      glm::mat4x4& viewProjection, unsigned int m_pointsCloudWorldTex,
                      unsigned int m_pointsCloudNormalTex)
{
	if (GUIScene::s_sdfCompute)
	{
		m_sdfCompute.compute(m_pointsCloudWorldTex, m_pointsCloudNormalTex, viewToWorld, worldToClip);
	}

	if (GUIScene::s_computeICPGPU)
	{
		m_icpCompute.compute(m_pointsCloudWorldTex, viewToWorld, viewProjection);
	}
}

//----------------------------------------------------------------------------------------------------------
void SDFScene::draw(ofCamera& camera)
{
	camera.begin();
	m_sdfCompute.drawOutline();
	if (GUIScene::s_sdfDrawRaytrace)
	{
		m_sdfCompute.drawRaymarch(camera);
	}
	if (GUIScene::s_sdfDrawSlice)
	{
		m_slice.draw(m_sdfCompute.getWorldInv(), m_sdfCompute.getTextureID(), 0);
	}

	camera.end();

	if (GUIScene::s_drawICPGPU)
	{
		FullScreenQuadRender::get().draw(m_icpCompute.getTexID(), GL_TEXTURE_2D);
	}
}
