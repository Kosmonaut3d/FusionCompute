#include "PointCloudScene.h"

#include "helper/dataStorageHelper.h"
#include "helper/fullScreenQuadRender.h"

//---------------------------------------------------
PointCloudScene::PointCloudScene()
    : m_bilateralBlurComp{}
    , m_pointCloudComp{}
    , m_pointCloudVis{}
    , m_pointCloudCPU_0{}
    , m_pointCloudCPU_1{}
    , m_icpCPU{}
    , m_texDepthRaw{}
    , m_texColorPtr{}
    , m_isPCL_0{false}
    , m_viewToWorldFirst{}
{
}

//----------------------------------------------------------------------------------------------------------
void PointCloudScene::setup(ofxKinect& kinect, glm::mat4x4 viewToWorld)
{
	m_viewToWorldFirst = viewToWorld;
	if (kinect.isConnected())
	{
		m_pointCloudComp.registerKinectData(kinect.getZeroPlaneDistance(), kinect.getZeroPlanePixelSize());

		m_pointCloudCPU_0.fillPointCloud(kinect, GUIScene::s_pointCloudDownscale, true, viewToWorld);
		m_pointCloudCPU_1.fillPointCloud(kinect, GUIScene::s_pointCloudDownscale, true, viewToWorld);
	}

	DataStorageHelper::loadData("depthRaw.bin", kinect.getRawDepthPixels().getData(), 640 * 480);
	m_texDepthRaw.allocate(kinect.getRawDepthPixels());
	m_texColorPtr = &kinect.getTexture();

	m_bilateralBlurComp.compute(m_texDepthRaw, GUIScene::s_bilateralBlurCompute);

	m_pointCloudComp.compute(m_bilateralBlurComp.getTextureID());
}

//----------------------------------------------------------------------------------------------------------
void PointCloudScene::update(bool kinectUpdate, ofxKinect& kinect, glm::mat4x4& viewToWorld, glm::mat4x4& worldToView,
                             glm::mat4x4& projection)
{
	if (kinectUpdate || GUIScene::s_pointCloudCPUForceUpdate)
	{
		m_texDepthRaw.loadData(kinect.getRawDepthPixels());
		m_bilateralBlurComp.compute(m_texDepthRaw, GUIScene::s_bilateralBlurCompute);

		if (GUIScene::s_computePointCloud)
		{
			m_pointCloudComp.compute(m_bilateralBlurComp.getTextureID());
		}

		if (GUIScene::s_computePointCloudCPU)
		{
			m_isPCL_0 = !m_isPCL_0;

			// Keep 2 PCL buffers
			if (m_isPCL_0)
			{
				m_pointCloudCPU_0.fillPointCloud(kinect, GUIScene::s_pointCloudDownscale, true, viewToWorld);
			}
			else
			{
				m_pointCloudCPU_1.fillPointCloud(kinect, GUIScene::s_pointCloudDownscale, true, viewToWorld);
			}
		}
	}

	if (GUIScene::s_computeICPCPU)
	{
		// GUIScene::s_computeICPCPU = false;

		auto& ptrNew = m_isPCL_0 ? m_pointCloudCPU_0 : m_pointCloudCPU_1;
		auto& ptrOld = m_isPCL_0 ? m_pointCloudCPU_1 : m_pointCloudCPU_0;

		glm::mat4x4 output;
		m_icpCPU.compute(ptrNew.getPoints(), ptrNew.getNormals(), ptrOld.getPoints(), ptrOld.getNormals(), worldToView,
		                 projection, output, GUIScene::s_pointCloudDownscale);

		worldToView = output;
		viewToWorld = glm::inverse(worldToView);
	}
}

//----------------------------------------------------------------------------------------------------------
void PointCloudScene::draw(ofCamera& camera, glm::mat4x4& viewToWorld, glm::mat4x4& worldToView,
                           glm::mat4x4& projection)
{
	camera.begin();

	ofEnableDepthTest();

	drawCameraOrientation(viewToWorld, worldToView, projection);

	if (GUIScene::s_drawPointCloud)
	{
		m_pointCloudVis.draw(m_pointCloudComp.getModelTextureID(), m_texColorPtr->getTextureData().textureID, false,
		                     camera.getModelViewProjectionMatrix());
	}

	if (GUIScene::SceneSelection::PointCloud == GUIScene::s_sceneSelection)
	{
		if (GUIScene::s_drawPointCloudCPU)
		{
			if (m_isPCL_0)
			{
				m_pointCloudCPU_0.draw(GUIScene::s_drawPointCloudNormCPU, viewToWorld);
			}
			else
			{
				m_pointCloudCPU_1.draw(GUIScene::s_drawPointCloudNormCPU, viewToWorld);
			}
		}

		if (GUIScene::s_drawPointCloudNorm)
		{
			FullScreenQuadRender::get().draw(m_pointCloudComp.getNormalTextureID(), GL_TEXTURE_2D);
		}
		else if (GUIScene::s_drawPointCloudTex)
		{
			FullScreenQuadRender::get().draw(m_pointCloudComp.getModelTextureID(), GL_TEXTURE_2D);
		}
	}

	camera.end();
}

//----------------------------------------------------------------------------------------------------------
void PointCloudScene::drawCameraOrientation(glm::mat4x4& viewToWorld, glm::mat4x4& worldToView, glm::mat4x4& projection)
{
	ofPushMatrix();
	auto      viewTransform     = m_viewToWorldFirst;
	auto      viewTransformCurr = viewToWorld;
	glm::vec4 zero(0, 0, 0, 1);
	glm::vec4 target(0, 0, -1, 1);
	ofSetColor(255, 0, 0);
	ofDrawArrow(viewTransform * zero, viewTransform * target, 0.02);
	ofSetColor(0, 255, 0);
	ofDrawArrow(viewTransformCurr * zero, viewTransformCurr * target, 0.03);

	glm::mat4 clipSpaceToWorld = glm::inverse(projection * worldToView);

	// Move into Clip Space - this matrix will transform anything we
	// draw from Clip Space to World Space
	ofMultMatrix(clipSpaceToWorld);

	// Anything we draw now is transformed from Clip Space back to World Space

	ofPushStyle();
	ofNoFill();
	ofDrawBox(0, 0, 0, 2.f, 2.f,
	          2.f); // In Clip Space, the frustum is standardised to be a box of dimensions -1,1 in each axis
	ofPopStyle();
	ofPopMatrix();
}
