#include "ofApp.h"
#include "helper/dataStorageHelper.h"
#include "helper/fullScreenQuadRender.h"

ofApp::ofApp()
    : ofBaseApp()
    , m_guiScene()
    , m_pointCloudScene()
    , m_blurScene{}
    , m_sdfScene{}
    , m_screenShotImage{}
    , m_kinectWorldToView{}
    , m_kinectViewToWorld{}
    , m_kinectProjection{}
// m_sdf(128, glm::vec3(-10, -10, -20), 20, 2),
{
	constexpr float fovy       = glm::radians(45.25); // Got this value by testing
	glm::mat4x4     projection = glm::perspective(fovy, 4.0f / 3.0f, 0.1f, 40.0f);

	glm::vec3   kinectOrigin = glm::vec3(0, 0, 0);
	auto        ori          = ofVec3f(kinectOrigin);
	auto        tar          = ofVec3f(kinectOrigin + glm::vec3(0, 0, -1));
	auto        upv          = ofVec3f(glm::vec3(0, 1, 0));
	ofMatrix4x4 view;
	view.makeLookAtViewMatrix(ori, tar, upv);

	m_kinectWorldToView = view;
	m_kinectProjection  = projection;
	m_kinectViewToWorld = view.getInverse();
}

//--------------------------------------------------------------
void ofApp::setup()
{

	ofSetVerticalSync(false);
	ofSetFrameRate(0);

	// IMPORTANT - read rect textures as normalized GL_TEXTURE_2D instead!
	ofDisableArbTex();

	ofDisableSmoothing();
	ofSetLineWidth(2);

	m_camera.setPosition(glm::vec3(0, 0, 0));
	m_camera.setTarget(glm::vec3(0, 0, -2)); // look forward
	m_camera.setDistance(2);
	m_camera.setFov(48.6f);
	m_camera.setNearClip(0.01); // 10 cm
	m_camera.setFarClip(1000);
	m_camera.setAspectRatio(4. / 3);
	m_camera.setTranslationKey(32); // space

	// enable depth->video image calibration
	m_kinect.setRegistration(true);
	m_kinect.init();
	m_kinect.open(); // opens first available kinect

	// print the intrinsic IR sensor values
	if (m_kinect.isConnected())
	{
		ofLogNotice() << "sensor-emitter dist: " << m_kinect.getSensorEmitterDistance() << "cm";
		ofLogNotice() << "sensor-camera dist:  " << m_kinect.getSensorCameraDistance() << "cm";
		ofLogNotice() << "zero plane pixel size: " << m_kinect.getZeroPlanePixelSize() << "mm";
		ofLogNotice() << "zero plane dist: " << m_kinect.getZeroPlaneDistance() << "mm";
	}

	m_depthImage.allocate(m_kinect.width, m_kinect.height, ofImageType::OF_IMAGE_GRAYSCALE);
	DataStorageHelper::loadImage("depth.bin", m_depthImage);

	m_guiScene.setup();
	m_pointCloudScene.setup(m_kinect, m_kinectViewToWorld);
	m_blurScene.setup(m_kinect);
	m_sdfScene.setup(m_kinect);
}

//--------------------------------------------------------------
void ofApp::exit()
{
	m_kinect.close();
}

//--------------------------------------------------------------
void ofApp::drawKinectPointCloud(ofxKinect& kinect)
{
	int    w = 640;
	int    h = 480;
	ofMesh mesh;
	mesh.setMode(OF_PRIMITIVE_POINTS);
	int step = 2;
	for (int y = 0; y < h; y += step)
	{
		for (int x = 0; x < w; x += step)
		{
			if (kinect.getDistanceAt(x, y) > 0)
			{
				mesh.addColor(kinect.getColorAt(x, y));
				mesh.addVertex(kinect.getWorldCoordinateAt(x, y));
			}
		}
	}
	glPointSize(3);
	ofPushMatrix();
	// the projected points are 'upside down' and 'backwards'
	ofScale(0.01, -0.01, -0.01);
	ofTranslate(0, 0, 0); // center the points a bit
	ofEnableDepthTest();
	mesh.drawVertices();
	ofDisableDepthTest();
	ofPopMatrix();
}

//--------------------------------------------------------------
void ofApp::drawFullScreenImage(ofImage& image)
{
	int width  = ofGetViewportWidth();
	int height = ofGetViewportHeight();

	float aspectImage = image.getWidth() / static_cast<float>(image.getHeight());
	float aspectView  = width / static_cast<float>(height);

	if (aspectView >= aspectImage)
	{
		width = static_cast<int>(aspectImage * height);
	}
	else
	{
		height = static_cast<int>(width / aspectImage);
	}

	image.draw(0, 0, width, height);
}

//--------------------------------------------------------------
void ofApp::update()
{

	m_kinect.update();

	const bool updateKinect = m_kinect.isFrameNew() && GUIScene::s_updateKinectData;

	// Update
	if (updateKinect) //&& !m_computeSDF && m_updateKinect)
	{
		GUIScene::s_isKinectDeliveringData = true;

		// TODO maybe use the kinect internal one instead, or make this a reference
		m_depthImage.setFromPixels(m_kinect.getDepthPixels());
		m_depthImage.update();
	}

	switch (GUIScene::s_sceneSelection)
	{
		case GUIScene::SceneSelection::Blur: {
			m_blurScene.update(updateKinect, m_kinect);
			break;
		}
		case GUIScene::SceneSelection::PointCloud: {
			m_pointCloudScene.update(updateKinect, m_kinect, m_kinectViewToWorld, m_kinectWorldToView, m_kinectProjection);
			break;
		}
		case GUIScene::SceneSelection::SDF: {
			m_pointCloudScene.update(updateKinect, m_kinect, m_kinectViewToWorld, m_kinectWorldToView,
			                         m_kinectProjection);
			m_sdfScene.update(updateKinect, m_kinect);
			break;
		}
		default: {

			break;
		}
	}
	m_guiScene.update();

	/*if (m_drawSDFAlgorithm)
	{
	    m_computeSDFAlgorithm.compute(m_depthRawTexture);
	}*/

	/*
	static int i = 0;
	if (m_computeSDF)
	{
	    const int batchsize = 2000;
	    //auto& mesh = m_pointCloud.getMesh();
	    const ofColor finishColor = ofColor::red;

	    if (m_pointCloud.getSize() > 0)
	    {
	        for (int j = 0; j < batchsize; j++)
	        {
	            int k = j + i;

	            if (k >= m_pointCloud.getSize())
	            {
	                i = 0;
	                m_computeSDF = false;

	                //m_sdf.storeData();

	                break;
	            }

	            //m_sdf.insertPoint(glm::vec3(m_pointCloud.getPoints()[k]), glm::vec3(0, 0, 0), 0.75f, 0.1f);
	            //auto color = mesh.getColor(k);
	            //mesh.setColor(k, color * ofColor::red);
	        }
	        i += batchsize;
	        m_buildProgress = 1.f * i / m_pointCloud.getSize();
	    }
	}*/
}

//--------------------------------------------------------------
void ofApp::draw()
{
	ofDisableDepthTest();
	ofBackground(GUIScene::s_backgroundColor);

	if (GUIScene::s_drawDepthBackground)
	{
		ofSetColor(ofColor::white * 0.2);
		FullScreenQuadRender::get().draw(m_depthImage.getTexture());
		// drawFullScreenImage(m_depthImage);
	}

	switch (GUIScene::s_sceneSelection)
	{
		case GUIScene::SceneSelection::Blur: {
			m_blurScene.draw();
			break;
		}
		case GUIScene::SceneSelection::PointCloud: {
			m_pointCloudScene.draw(m_camera, m_kinectViewToWorld, m_kinectWorldToView, m_kinectProjection);
			break;
		}
		case GUIScene::SceneSelection::SDF: {
			m_pointCloudScene.draw(m_camera, m_kinectViewToWorld, m_kinectWorldToView, m_kinectProjection);
			m_sdfScene.draw();
			break;
		}
		default: {

			break;
		}
	}
	/*
	ofDisableDepthTest();

	if (m_drawSDFAlgorithm)
	{
	    //ofSetColor(ofColor::white);
	    //m_computeSDFAlgorithm.draw(m_depthRawTexture);

	    m_pointCloudComp.draw(m_computeSDFAlgorithm.getTextureID(), m_kinect.getTexture().getTextureData().textureID,
	false, m_camera.getModelViewProjectionMatrix(), m_pclSizeValue);

	}

	m_camera.begin();
	ofEnableDepthTest();

	//m_sdf.drawOutline();
	if (m_drawSDF)
	{
	    //m_sdf.drawRaymarch(m_camera);
	}
	if (m_drawSlice)
	{
	    //ofSetColor(ofColor::red);
	    //m_slice.draw(m_sdf.getInvWorld(), m_sdf.getTextureID(), m_computeSDFAlgorithm.getTextureID());
	}

	m_camera.end(); */

	// Draw GUI
	m_guiScene.draw(m_camera);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
	if (key == 'p')
	{
		static int i = 0;
		m_screenShotImage.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
		std::string name = "screenshot" + std::to_string(++i) + std::string(".png");
		m_screenShotImage.save(name);
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key)
{
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y)
{
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button)
{
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button)
{
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button)
{
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y)
{
}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y)
{
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h)
{
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg)
{
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo)
{
}
