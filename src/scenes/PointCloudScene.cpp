#include "PointCloudScene.h"

#include "helper/dataStorageHelper.h"
#include "helper/fullScreenQuadRender.h"

//---------------------------------------------------
PointCloudScene::PointCloudScene()
    : m_bilateralBlurComp{} 
	,m_pointCloudComp{}
    , m_pointCloudVis{}
    , m_pointCloudCPU_0{}
    , m_pointCloudCPU_1{}
    , m_icpCPU{}
    , m_texDepthRaw{}
    , m_texColorPtr{}
    , m_isPCL_0{false}
    , m_kinectViewFirst{}
    , m_kinectViewCurrent{}
    , m_kinectViewToWorldCurrent{}
    , m_kinectProjection{}
{
	constexpr float fovy       = glm::radians(45.25); // Got this value by testing
	glm::mat4x4     projection = glm::perspective(fovy, 4.0f / 3.0f, 0.1f, 1000.0f);

	glm::vec3   kinectOrigin = glm::vec3(0, 0, 0);
	auto        ori          = ofVec3f(kinectOrigin);
	auto        tar          = ofVec3f(kinectOrigin + glm::vec3(0, 0, -1));
	auto        upv          = ofVec3f(glm::vec3(0, 1, 0));
	ofMatrix4x4 view;
	view.makeLookAtViewMatrix(ori, tar, upv);

	m_kinectViewFirst          = view;
	m_kinectViewCurrent        = view;
	m_kinectProjection         = projection;
	m_kinectViewToWorldCurrent = view.getInverse();
}

//----------------------------------------------------------------------------------------------------------
void PointCloudScene::setup(ofxKinect& kinect)
{
	float zeroPlaneDist  = 0.104;
	float zeroPlanePixel = 120;
	if (kinect.isConnected())
	{
		m_pointCloudComp.registerKinectData(kinect.getZeroPlaneDistance(), kinect.getZeroPlanePixelSize());
	}

	DataStorageHelper::loadData("depthRaw.bin", kinect.getRawDepthPixels().getData(), 640 * 480);
	m_texDepthRaw.allocate(kinect.getRawDepthPixels());
	m_texColorPtr = &kinect.getTexture();

	m_bilateralBlurComp.compute(m_texDepthRaw, GUIScene::s_bilateralBlurCompute);

	m_pointCloudComp.compute(m_bilateralBlurComp.getTextureID());
	m_pointCloudCPU_0.fillPointCloud(kinect, GUIScene::s_pointCloudDownscale, true, m_kinectViewToWorldCurrent);
	m_pointCloudCPU_1.fillPointCloud(kinect, GUIScene::s_pointCloudDownscale, true, m_kinectViewToWorldCurrent);
}

//----------------------------------------------------------------------------------------------------------
void PointCloudScene::update(bool kinectUpdate, ofxKinect& kinect)
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
				m_pointCloudCPU_0.fillPointCloud(kinect, GUIScene::s_pointCloudDownscale, true,
				                                 m_kinectViewToWorldCurrent);
			}
			else
			{
				m_pointCloudCPU_1.fillPointCloud(kinect, GUIScene::s_pointCloudDownscale,
				                                 true, m_kinectViewToWorldCurrent);
			}
		}
	}

	if (GUIScene::s_computeICPCPU)
	{
		// GUIScene::s_computeICPCPU = false;

		auto& ptrNew = m_isPCL_0 ? m_pointCloudCPU_0 : m_pointCloudCPU_1;
		auto& ptrOld = m_isPCL_0 ? m_pointCloudCPU_1 : m_pointCloudCPU_0;

		glm::mat4x4 output;
		m_icpCPU.compute(ptrNew.getPoints(), ptrNew.getNormals(), ptrOld.getPoints(), ptrOld.getNormals(),
		                 m_kinectViewCurrent, m_kinectProjection, output, GUIScene::s_pointCloudDownscale);

		m_kinectViewCurrent = output;
		m_kinectViewToWorldCurrent = glm::inverse(m_kinectViewCurrent);
	}
}

//----------------------------------------------------------------------------------------------------------
void PointCloudScene::draw(ofCamera& camera)
{
	camera.begin();

	ofEnableDepthTest();

	drawOutline();

	drawCameraOrientation();

	if (GUIScene::s_drawPointCloud)
	{
		m_pointCloudVis.draw(m_pointCloudComp.getModelTextureID(), m_texColorPtr->getTextureData().textureID, false,
		                     camera.getModelViewProjectionMatrix());
	}

	if (GUIScene::s_drawPointCloudCPU)
	{
		if (m_isPCL_0)
		{
			m_pointCloudCPU_0.draw(GUIScene::s_drawPointCloudNormCPU, glm::inverse(m_kinectViewCurrent));
		}
		else
		{
			m_pointCloudCPU_1.draw(GUIScene::s_drawPointCloudNormCPU, glm::inverse(m_kinectViewCurrent));
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

	camera.end();
}

//----------------------------------------------------------------------------------------------------------
void PointCloudScene::drawOutline()
{
	glm::vec3 origin    = glm::vec3(-2, -2, -4);
	float     scale     = 4;
	float     scalehalf = scale / 2;

	ofPushStyle();
	ofGetCurrentRenderer()->setFillMode(ofFillFlag::OF_OUTLINE);
	ofSetColor(255, 0, 0);
	ofDrawBox(glm::vec3(0, 0, 0), 1, 1, 1);
	ofSetColor(200, 100, 200);
	ofDrawBox(origin + glm::vec3(scalehalf, scalehalf, scalehalf), scale, scale, scale);
	ofPopStyle();
}

//----------------------------------------------------------------------------------------------------------
void PointCloudScene::drawTest(ofxKinect& kinect)
{
	static float fov_y      = 45.25;
	float        fovy       = glm::radians(fov_y); // 48.6);
	glm::mat4x4  projection = glm::perspective(fovy, 4.0f / 3.0f, 0.1f, 1000.0f);

	glm::vec3   kinectOrigin = glm::vec3(0, 0, 0);
	glm::mat4x4 view         = glm::lookAt(kinectOrigin, kinectOrigin + glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));

	int x = 500;
	int y = 100;

	glm::vec3 pointWorld = kinect.getWorldCoordinateAt(500, 100) * glm::vec3(1, -1, -1) * 0.01;
	ofDrawSphere(pointWorld, .01);

	ofVec4f clipSpacePos = projection * glm::vec4(pointWorld, 1);

	if (clipSpacePos.w <= 0)
	{
	}

	glm::vec3 clipxyz = ofVec3f(clipSpacePos);
	glm::vec3 ndc     = clipxyz / clipSpacePos.w;
	if (ndc.x < -1 || ndc.x > 1 || ndc.y < -1 || ndc.y > 1)
	{
	}

	// OPENGL
	ndc.y        = -ndc.y;
	float x_proj = (ndc.x + 1) * 320;
	float y_proj = (ndc.y + 1) * 240;

	float err = 1 - x_proj / x;
	fov_y -= err;

	bool result = true;

	// ofVec3f output = ofVec3f(ndc_x, ndc_y, ndc_z) * glm::inverse(camera.getModelViewProjectionMatrix());
	// ofDrawSphere(output, .01);
}

//----------------------------------------------------------------------------------------------------------
void PointCloudScene::drawCameraOrientation()
{
	auto      viewTransform     = glm::inverse(m_kinectViewFirst);
	auto      viewTransformCurr = glm::inverse(m_kinectViewCurrent);
	glm::vec4 zero(0, 0, 0, 1);
	glm::vec4 target(0, 0, -1, 1);
	ofSetColor(255, 0, 0);
	ofDrawArrow(viewTransform * zero, viewTransform * target, 0.02);
	ofSetColor(0, 255, 0);
	ofDrawArrow(viewTransformCurr * zero, viewTransformCurr * target, 0.03);
}
