#include "SDFScene.h"

#include "helper/fullScreenQuadRender.h"

/// <summary>
/// Scene that renders the SDF and can set up the SDF ICP algorithm
/// </summary>
SDFScene::SDFScene()
    : m_sdfCompute{glm::vec3(-2, -2, -3), GUIScene::s_sdfResolution, 4}
    , m_sdfCPU{GUIScene::s_sdfResolution, glm::vec3(-2, -2, -4), 4, 2}
    , m_slice(glm::vec3(0, 0, -1), 2)
    , m_icpCompute{}
    , m_kinectColorTexPtr{}
{
}

/// <summary>
/// Setup our texture pointer at the Kinect RGB stream
/// </summary
void SDFScene::setup(ofxKinect& kinect)
{
	m_kinectColorTexPtr = &kinect.getTexture();
}

//----------------------------------------------------------------------------------------------------------
void SDFScene::update(bool kinectUpdate, ofxKinect& kinect, glm::mat4x4& viewToWorld, glm::mat4x4& worldToView,
                      glm::mat4x4& projection, unsigned int m_pointsCloudWorldTexNew,
                      unsigned int m_pointsCloudNormalTexNew, unsigned int m_pointsCloudWorldTexOld,
                      unsigned int m_pointsCloudNormalTexOld)
{
	if (!kinectUpdate)
	{
		return;
	}

	// Compute ICP
	if (GUIScene::s_ICP_GPU_compute)
	{
		glm::mat4x4 viewToWorldIt =
		    m_icpCompute.compute(m_pointsCloudWorldTexNew, m_pointsCloudNormalTexNew, m_pointsCloudWorldTexOld,
		                         m_pointsCloudNormalTexOld, viewToWorld, projection, m_sdfCompute);

		// Apply the calculated transformation to the camera
		if (GUIScene::s_ICP_applyTransformation)
		{
			worldToView = glm::inverse(viewToWorldIt);
			viewToWorld = viewToWorldIt;
		}
	}

	// Calculate the new SDF information
	if (GUIScene::s_sdfCompute)
	{
		m_sdfCompute.compute(m_pointsCloudWorldTexNew, m_pointsCloudNormalTexNew,
		                     m_kinectColorTexPtr->getTextureData().textureID, viewToWorld, projection * worldToView);
	}
}

//----------------------------------------------------------------------------------------------------------
void SDFScene::draw(ofCamera& camera)
{
	camera.begin();
	if (GUIScene::s_drawHelpers)
	{
		m_sdfCompute.drawOutline();
	}
	if (GUIScene::s_sdfDrawRaytrace)
	{
		m_sdfCompute.drawRaymarch(camera);
	}
	if (GUIScene::s_sdfDrawSlice)
	{
		m_slice.draw(m_sdfCompute.getSDFBaseTransformation(), m_sdfCompute.getTextureID(), 0);
	}

	camera.end();

	// Draw the correspondence frame from the ICP computation
	if (GUIScene::s_ICP_GPU_drawDebug)
	{
		FullScreenQuadRender::get().draw(m_icpCompute.getTexID(), GL_TEXTURE_2D);
	}
}
