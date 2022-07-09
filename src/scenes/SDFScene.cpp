#include "SDFScene.h"

#include "helper/fullScreenQuadRender.h"

//---------------------------------------------------
SDFScene::SDFScene()
    : m_sdfCompute{glm::vec3(-1, -1, -2), GUIScene::s_sdfResolution, 2}
    , m_sdfCPU{GUIScene::s_sdfResolution, glm::vec3(-2, -2, -4), 4, 2}
    , m_slice(glm::vec3(0, 0, -1), 2)
    , m_icpCompute{}
    , m_kinectColorTexPtr{}
{
}

//----------------------------------------------------------------------------------------------------------
void SDFScene::setup(ofxKinect& kinect)
{
	m_kinectColorTexPtr = &kinect.getTexture();
}

//----------------------------------------------------------------------------------------------------------
void SDFScene::update(bool kinectUpdate, ofxKinect& kinect, glm::mat4x4& worldToClip, glm::mat4x4& viewToWorld,
                      glm::mat4x4& worldToView, glm::mat4x4& projection, unsigned int m_pointsCloudWorldTexNew,
                      unsigned int m_pointsCloudNormalTexNew, unsigned int m_pointsCloudWorldTexOld,
                      unsigned int m_pointsCloudNormalTexOld)
{
	if (!kinectUpdate)
	{
		return;
	}
	if (GUIScene::s_sdfCompute)
	{
		m_sdfCompute.compute(m_pointsCloudWorldTexNew, m_pointsCloudNormalTexNew,
		                     m_kinectColorTexPtr->getTextureData().textureID, viewToWorld, worldToClip);
	}

	if (GUIScene::s_sdfExpand)
	{
		m_sdfCompute.computeExpandSDF();
	}

	if (GUIScene::s_ICP_GPU_compute)
	{
		glm::mat4x4 viewToWorldIt =
		    m_icpCompute.compute(m_pointsCloudWorldTexNew, m_pointsCloudNormalTexNew, m_pointsCloudWorldTexOld,
		                         m_pointsCloudNormalTexOld, viewToWorld, projection, m_sdfCompute);

		if (GUIScene::s_ICP_applyTransformation)
		{
			worldToView = glm::inverse(viewToWorldIt);
			viewToWorld = viewToWorldIt;
		}
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
		m_slice.draw(m_sdfCompute.getSDFBaseTransformation(), m_sdfCompute.getTextureID(), 0);
	}

	camera.end();

	if (GUIScene::s_ICP_GPU_drawDebug)
	{
		FullScreenQuadRender::get().draw(m_icpCompute.getTexID(), GL_TEXTURE_2D);
	}
}
