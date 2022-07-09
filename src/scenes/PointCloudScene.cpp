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
{
}

//----------------------------------------------------------------------------------------------------------
void PointCloudScene::setup(ofxKinect& kinect, glm::mat4x4 viewToWorld)
{
	if (kinect.isConnected())
	{
		m_pointCloudComp.registerKinectData(kinect.getZeroPlaneDistance(), kinect.getZeroPlanePixelSize());

		m_pointCloudCPU_0.fillPointCloud(kinect, GUIScene::s_PCL_CPU_downscale, true, glm::mat4() /*viewToWorld*/);
		m_pointCloudCPU_1.fillPointCloud(kinect, GUIScene::s_PCL_CPU_downscale, true, glm::mat4() /*viewToWorld*/);
	}

	DataStorageHelper::loadData("depthRaw.bin", kinect.getRawDepthPixels().getData(), 640 * 480);
	m_texDepthRaw.allocate(kinect.getRawDepthPixels());
	m_texColorPtr = &kinect.getTexture();

	m_bilateralBlurComp.compute(m_texDepthRaw, GUIScene::s_bilateralBlurCompute);

	m_pointCloudComp.compute(m_bilateralBlurComp.getTextureID(), true);
}

//----------------------------------------------------------------------------------------------------------
void PointCloudScene::update(bool kinectUpdate, ofxKinect& kinect, glm::mat4x4& viewToWorld, glm::mat4x4& worldToView,
                             glm::mat4x4& projection, bool isFrame0)
{
	if (kinectUpdate || GUIScene::s_PCL_CPU_forceUpdate)
	{
		m_texDepthRaw.loadData(kinect.getRawDepthPixels());

		if (GUIScene::s_PCL_GPU_compute)
		{
			m_bilateralBlurComp.compute(m_texDepthRaw, GUIScene::s_bilateralBlurCompute);
			m_pointCloudComp.compute(m_bilateralBlurComp.getTextureID(), isFrame0);
		}

		if (GUIScene::s_PCL_CPU_compute)
		{
			// Keep 2 PCL buffers
			if (isFrame0)
			{
				m_pointCloudCPU_0.fillPointCloud(kinect, GUIScene::s_PCL_CPU_downscale, true,
				                                 glm::mat4() /*viewToWorld*/);
			}
			else
			{
				m_pointCloudCPU_1.fillPointCloud(kinect, GUIScene::s_PCL_CPU_downscale, true, glm::mat4());
			}
		}

		if (GUIScene::s_ICP_CPU_compute)
		{
			// GUIScene::s_computeICPCPU = false;

			auto& ptrNew = isFrame0 ? m_pointCloudCPU_0 : m_pointCloudCPU_1;
			auto& ptrOld = isFrame0 ? m_pointCloudCPU_1 : m_pointCloudCPU_0;

			glm::mat4x4 output;
			m_icpCPU.compute(ptrNew.getPoints(), ptrNew.getNormals(), ptrOld.getPoints(), ptrOld.getNormals(),
			                 worldToView, projection, output, GUIScene::s_PCL_CPU_downscale);

			worldToView = output;
			viewToWorld = glm::inverse(worldToView);
		}
	}
}

//----------------------------------------------------------------------------------------------------------
void PointCloudScene::draw(ofCamera& camera, glm::mat4x4& viewToWorld, glm::mat4x4& worldToView,
                           glm::mat4x4& projection, bool isFrame0)
{
	camera.begin();

	ofEnableDepthTest();

	if (GUIScene::s_PCL_GPU_draw)
	{
		m_pointCloudVis.draw(m_pointCloudComp.getModelTextureID(isFrame0), m_texColorPtr->getTextureData().textureID,
		                     false, camera.getModelViewProjectionMatrix() * viewToWorld);
	}

	if (GUIScene::SceneSelection::PointCloud == GUIScene::s_sceneSelection)
	{
		if (GUIScene::s_PCL_CPU_draw)
		{
			ofPushMatrix();
			ofMultMatrix(viewToWorld);
			if (isFrame0)
			{
				m_pointCloudCPU_0.draw(GUIScene::s_PCL_CPU_debugDrawNormals, viewToWorld);
			}
			else
			{
				m_pointCloudCPU_1.draw(GUIScene::s_PCL_CPU_debugDrawNormals, viewToWorld);
			}
			ofPopMatrix();
		}

		if (GUIScene::s_PCL_GPU_debugDrawNormals)
		{
			FullScreenQuadRender::get().draw(m_pointCloudComp.getNormalTextureID(isFrame0), GL_TEXTURE_2D);
		}
		else if (GUIScene::s_PCL_GPU_debugDrawWorld)
		{
			FullScreenQuadRender::get().draw(m_pointCloudComp.getModelTextureID(isFrame0), GL_TEXTURE_2D);
		}
	}

	camera.end();
}

//----------------------------------------------------------------------------------------------------------
unsigned int PointCloudScene::getPCLWorld(bool isFrame0)
{
	return m_pointCloudComp.getModelTextureID(isFrame0);
}

//----------------------------------------------------------------------------------------------------------
unsigned int PointCloudScene::getPCLNormal(bool isFrame0)
{
	return m_pointCloudComp.getNormalTextureID(isFrame0);
}
