#include "PointCloudScene.h"
#include "helper/dataStorageHelper.h"

//---------------------------------------------------
PointCloudScene::PointCloudScene() : m_pointCloudComp{}, m_pointCloudVis{}, m_texDepthRaw{}, m_texColorPtr{}
{
	
}

//---------------------------------------------------
void PointCloudScene::setup(ofxKinect& kinect)
{
	m_pointCloudComp.registerKinectData(kinect.getZeroPlaneDistance(), kinect.getZeroPlanePixelSize());

	DataStorageHelper::loadData("depthRaw.bin", kinect.getRawDepthPixels().getData(), 640 * 480);
	m_texDepthRaw.allocate(kinect.getRawDepthPixels());
	m_texColorPtr = &kinect.getTexture();
}

//---------------------------------------------------
void PointCloudScene::update(bool kinectUpdate, ofxKinect& kinect)
{
	if (kinectUpdate && GUIScene::s_computePointCloud)
	{
		m_texDepthRaw.loadData(kinect.getRawDepthPixels());
		m_pointCloudComp.compute(m_texDepthRaw);
	}
}

//---------------------------------------------------
void PointCloudScene::draw(ofCamera& camera)
{
	camera.begin();
	glm::vec3 origin = glm::vec3(-10, -10, -20);
	float scale = 20;
	float scalehalf = scale / 2;

	ofEnableDepthTest();
	ofPushStyle();
	ofGetCurrentRenderer()->setFillMode(ofFillFlag::OF_OUTLINE);
	ofSetColor(255, 0, 0);
	ofDrawBox(glm::vec3(0,0,0), 1,1,1);
	ofSetColor(200, 100, 200);
	ofDrawBox(origin + glm::vec3(scalehalf, scalehalf, scalehalf), scale, scale, scale);
	ofPopStyle();

	m_pointCloudVis.draw(m_pointCloudComp.getTextureID(), m_texColorPtr->getTextureData().textureID, false, camera.getModelViewProjectionMatrix(), 10.0f);

	camera.end();
}


