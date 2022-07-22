#include "ofApp.h"
#include "helper/dataStorageHelper.h"
#include "helper/fullScreenQuadRender.h"

/// <summary>
/// The main application that runs the update and draw loops
/// </summary>
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
    , m_kinectViewToWorld_Init{}
    , m_isFrame0{false}
{
	// Set up the Kinect Virtual Camera parameters
	constexpr float fovy       = glm::radians(45.25); // Got this value by testing
	glm::mat4x4     projection = glm::perspective(fovy, 4.0f / 3.0f, 0.1f, 40.0f);

	// Camera origin = 0 at the beginning
	glm::vec3   kinectOrigin = glm::vec3(0, 0, 0);
	auto        ori          = ofVec3f(kinectOrigin);
	auto        tar          = ofVec3f(kinectOrigin + glm::vec3(0, 0, -1));
	auto        upv          = ofVec3f(glm::vec3(0, 1, 0));
	ofMatrix4x4 view;
	view.makeLookAtViewMatrix(ori, tar, upv);

	m_kinectWorldToView = view;
	m_kinectProjection  = projection;
	m_kinectViewToWorld = view.getInverse();

	// Initial orientation, for visualization
	m_kinectViewToWorld_Init = m_kinectViewToWorld;
}

/// <summary>
/// Initial setup functionality
/// </summary>
void ofApp::setup()
{
	// Unlimited framerate
	ofSetVerticalSync(false);
	ofSetFrameRate(0);

	// IMPORTANT - read rect textures as normalized GL_TEXTURE_2D instead!
	ofDisableArbTex();

	// No antialising
	ofDisableSmoothing();
	ofSetLineWidth(2);

	m_camera.setPosition(glm::vec3(0, 0, 0));
	m_camera.setTarget(glm::vec3(0, 0, -2)); // look forward
	m_camera.setDistance(2);
	m_camera.setFov(45.25f);
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

/// <summary>
/// This function is called every frame before draw(). All non-visual computations go in here.
/// </summary>
void ofApp::update()
{
	// Reset timers
	if (GUIScene::s_resetView)
	{
		m_kinectViewToWorld   = m_kinectViewToWorld_Init;
		m_kinectWorldToView   = glm::inverse(m_kinectViewToWorld);
		GUIScene::s_resetView = false;
	}

	// Trigger the kinect to update the video stream
	m_kinect.update();

	// Check if the kinect has new data (can be cancelled by GUI)
	const bool updateKinect = m_kinect.isFrameNew() && GUIScene::s_updateKinectData;

	// Update new data
	if (updateKinect)
	{
		GUIScene::s_isKinectDeliveringData = true;

		// For ping pong shaders
		m_isFrame0 = !m_isFrame0;

		// Update the depth map texture with values from the kinect stream
		m_depthImage.setFromPixels(m_kinect.getDepthPixels());
		m_depthImage.update();

		// Reset timers
		GUIScene::s_bilateralBlur_measureComputeTime = 0;
		GUIScene::s_PCL_GPU_measuredComputeTime      = 0;
		GUIScene::s_sdfMeasuredComputeTime           = 0;
		GUIScene::s_ICP_correspondenceMeasureTime    = 0;
		GUIScene::s_ICP_GPU_reductionMeasureTime     = 0;
		GUIScene::s_ICP_CPU_solveSystemMeasureTime   = 0;
	}

	// scene selection based on GUI
	switch (GUIScene::s_sceneSelection)
	{
		case GUIScene::SceneSelection::Blur: {
			m_blurScene.update(updateKinect, m_kinect);
			break;
		}
		case GUIScene::SceneSelection::PointCloud: {
			m_pointCloudScene.update(updateKinect, m_kinect, m_kinectViewToWorld, m_kinectWorldToView,
			                         m_kinectProjection, m_isFrame0);
			break;
		}
		case GUIScene::SceneSelection::SDF: {
			m_pointCloudScene.update(updateKinect, m_kinect, m_kinectViewToWorld, m_kinectWorldToView,
			                         m_kinectProjection, m_isFrame0);

			auto PCLWorldNew  = m_pointCloudScene.getPCLWorld(m_isFrame0);
			auto PCLWorldOld  = m_pointCloudScene.getPCLWorld(!m_isFrame0);
			auto PCLNormalNew = m_pointCloudScene.getPCLNormal(m_isFrame0);
			auto PCLNormalOld = m_pointCloudScene.getPCLNormal(!m_isFrame0);

			m_sdfScene.update(updateKinect, m_kinect, m_kinectViewToWorld, m_kinectWorldToView, m_kinectProjection,
			                  PCLWorldNew, PCLNormalNew, PCLWorldOld, PCLNormalOld);
			break;
		}
		default: {

			break;
		}
	}
	m_guiScene.update();
}

/// <summary>
/// This function implements all visual computations
/// </summary>
void ofApp::draw()
{
	// No depth testing
	ofDisableDepthTest();

	// Update background color
	ofBackground(GUIScene::s_backgroundColor);

	// Draw Kinect Depth as background optionally
	if (GUIScene::s_drawDepthBackground)
	{
		ofSetColor(ofColor::white * 0.2);
		FullScreenQuadRender::get().draw(m_depthImage.getTexture());
	}

	switch (GUIScene::s_sceneSelection)
	{
		case GUIScene::SceneSelection::Blur: {
			m_blurScene.draw();
			break;
		}
		case GUIScene::SceneSelection::PointCloud: {
			m_pointCloudScene.draw(m_camera, m_kinectViewToWorld, m_kinectWorldToView, m_kinectProjection, m_isFrame0);

			if (GUIScene::s_drawHelpers)
			{
				m_camera.begin();
				drawCameraOrientation(m_kinectViewToWorld, m_kinectWorldToView, m_kinectProjection);
				m_camera.end();
			}
			break;
		}
		case GUIScene::SceneSelection::SDF: {
			m_sdfScene.draw(m_camera);

			m_pointCloudScene.draw(m_camera, m_kinectViewToWorld, m_kinectWorldToView, m_kinectProjection, m_isFrame0);

			if (GUIScene::s_drawHelpers)
			{
				m_camera.begin();
				drawCameraOrientation(m_kinectViewToWorld, m_kinectWorldToView, m_kinectProjection);
				m_camera.end();
			}
			break;
		}
		default: {

			break;
		}
	}

	// Draw GUI
	m_guiScene.draw(m_camera);
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

/// <summary>
/// Will draw the Kinect Camera frustum
/// </summary>
void ofApp::drawCameraOrientation(glm::mat4x4& viewToWorld, glm::mat4x4& worldToView, glm::mat4x4& projection)
{
	ofPushMatrix();
	auto      viewTransform        = m_kinectViewToWorld_Init;
	auto      viewTransformCurrent = viewToWorld;
	glm::vec4 zero(0, 0, 0, 1);
	glm::vec4 target(0, 0, -1, 1);
	ofSetColor(255, 0, 0);

	// Transforming vector zero with the Kinect view transformation matrix will result in the kamera origin.
	ofDrawArrow(viewTransform * zero, viewTransform * target, 0.02);
	ofSetColor(0, 255, 0);
	ofDrawArrow(viewTransformCurrent * zero, viewTransformCurrent * target, 0.03);

	glm::mat4 clipSpaceToWorld = glm::inverse(projection * worldToView);

	// Move into Clip Space - this matrix will transform anything we
	// draw from Clip Space to World Space
	ofMultMatrix(clipSpaceToWorld);

	// Anything we draw now is transformed from Clip Space back to World Space
	ofPushStyle();
	ofNoFill();
	// In Clip Space, the frustum is standardised to be a box of dimensions -1,1 in each axis
	ofDrawBox(0, 0, 0, 2.f, 2.f, 2.f);
	ofPopStyle();
	ofPopMatrix();
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
