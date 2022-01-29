#include "ofApp.h"

ofApp::ofApp() : ofBaseApp(),
m_sdf(128, glm::vec3(-10, -10, -20), 20, 2),
m_computeSDFAlgorithm(),
m_slice(glm::vec3(0, 0, -10), 20),
m_depthMultipy{ 2.0f },
m_minDepthGrid{ 2.0f },
m_renderMode{ RenderMode::SDF },
m_computeSDF{ false },
m_drawSlice{ false },
m_drawDepthBackground{ true },
m_drawSDF{ false },
m_drawPointCloud{ false },
m_drawSDFAlgorithm{ false },
m_buildProgress{0.0f},
m_backgroundColor{ofColor::white*0.2},
m_floatValue{0.1f},
m_sdfResolutionExp{6},
m_sdfResolution(pow(2, m_sdfResolutionExp))
{
}

//--------------------------------------------------------------
void ofApp::setup() {
	ofSetVerticalSync(false);
	ofSetFrameRate(0);

	ofDisableSmoothing();
	ofSetLineWidth(2);

	ofBackground(70, 70, 70);
	ofEnableDepthTest();

	glEnable(GL_TEXTURE_3D);

	m_camera.setPosition(glm::vec3(0, 0, 0));
	m_camera.setTarget(glm::vec3(0, 0, -1)); // look forward
	m_camera.setDistance(10);
	m_camera.setNearClip(1);
	m_camera.setFarClip(1000);
	m_camera.setTranslationKey(32); // space


	// enable depth->video image calibration
	m_kinect.setRegistration(true);
	m_kinect.init();
	m_kinect.open();// opens first available kinect

	// print the intrinsic IR sensor values
	if (m_kinect.isConnected()) {
		ofLogNotice() << "sensor-emitter dist: " << m_kinect.getSensorEmitterDistance() << "cm";
		ofLogNotice() << "sensor-camera dist:  " << m_kinect.getSensorCameraDistance() << "cm";
		ofLogNotice() << "zero plane pixel size: " << m_kinect.getZeroPlanePixelSize() << "mm";
		ofLogNotice() << "zero plane dist: " << m_kinect.getZeroPlaneDistance() << "mm";
	}

	m_depthImage.allocate(m_kinect.width, m_kinect.height, ofImageType::OF_IMAGE_GRAYSCALE);

	// IMGUI
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	m_gui.setup(); 
}

//--------------------------------------------------------------
void ofApp::exit()
{
	m_kinect.close();
}

//--------------------------------------------------------------
void ofApp::drawKinectPointCloud(ofxKinect& kinect) {
	int w = 640;
	int h = 480;
	ofMesh mesh;
	mesh.setMode(OF_PRIMITIVE_POINTS);
	int step = 2;
	for (int y = 0; y < h; y += step) {
		for (int x = 0; x < w; x += step) {
			if (kinect.getDistanceAt(x, y) > 0) {
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
	int width = ofGetViewportWidth();
	int height = ofGetViewportHeight();

	float aspectImage = image.getWidth() / static_cast<float>(image.getHeight());
	float aspectView = width / static_cast<float>(height);

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

void ofApp::drawGUI()
{
	m_gui.begin();

	// 1. Show a simple window
	{
		ImGui::SetWindowPos(ofVec2f(0, 0), ImGuiCond_FirstUseEver);
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		if (ImGui::SliderFloat("Float", &m_floatValue, -1.0f, 1.0f))
		{
			m_slice.setPos(glm::vec3(m_floatValue*10, 0, -10));
		};

		//this will change the app background color
		ImGui::ColorEdit3("Background Color", (float*)&m_backgroundColor);

		ImGui::Checkbox("Draw Slice", &m_drawSlice);
		ImGui::Checkbox("Draw Kinect Depth", &m_drawDepthBackground);
		ImGui::Checkbox("Draw SDF", &m_drawSDF);
		ImGui::Checkbox("Draw PCL", &m_drawPointCloud);
		ImGui::Checkbox("Draw SDF Compute", &m_drawSDFAlgorithm);

		if (ImGui::SliderInt("SDF resolution", &m_sdfResolutionExp, 3, 8))
		{
			m_sdfResolution = pow(2, m_sdfResolutionExp);
			m_sdf.setResolution(m_sdfResolution);
		} ImGui::SameLine();
		ImGui::Text("%d", m_sdfResolution);

		if (ImGui::Button("Compute SDF "))
		{
			m_sdf.resetData();
			m_computeSDF = !m_computeSDF;
		} ImGui::SameLine();

		ImGui::ProgressBar(m_buildProgress);

		if (ImGui::Button("Apply SDF tex"))
		{
			m_sdf.update3dTexture();

		}
	}

	if (ImGui::GetIO().WantCaptureMouse)
	{
		m_camera.disableMouseInput();
	}
	else
	{
		m_camera.enableMouseInput();
	}

	// endcall
	m_gui.end();
}

//--------------------------------------------------------------
void ofApp::update() {

	m_kinect.update();

	if (m_kinect.isFrameNew() && !m_computeSDF)
	{
		m_depthImage.setFromPixels(m_kinect.getDepthPixels());
		m_depthImage.update();
		m_pointCloud.fillPointCloud(m_kinect, 2);
	}

	if (m_drawSDFAlgorithm)
	{
		m_computeSDFAlgorithm.compute();
	}

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

					m_sdf.storeData();

					break;
				}

				m_sdf.insertPoint(glm::vec3(m_pointCloud.getPoints()[k]), glm::vec3(0, 0, 0), 0.75f, 0.1f);
				//auto color = mesh.getColor(k);
				//mesh.setColor(k, color * ofColor::red);
			}
			i += batchsize;
			m_buildProgress = 1.f * i / m_pointCloud.getSize();
		}
	}
}

//--------------------------------------------------------------
void ofApp::draw()
{
	switch (m_renderMode)
	{

	case RenderMode::SDF:
	{
		ofBackground(m_backgroundColor);
		ofDisableDepthTest();
		if (m_drawDepthBackground)
		{
			ofSetColor(ofColor::white * 0.2);
			drawFullScreenImage(m_depthImage);
		}

		if (m_drawSDFAlgorithm)
		{
			ofSetColor(ofColor::white);
			m_computeSDFAlgorithm.draw();
		}

		m_camera.begin();
		ofEnableDepthTest();

		m_sdf.drawOutline();
		if (m_drawSDF)
		{
			m_sdf.drawRaymarch(m_camera);
		}
		if (m_drawPointCloud)
		{
			m_pointCloud.draw();
		}
		if (m_drawSlice)
		{
			ofSetColor(ofColor::red);
			m_slice.draw(m_sdf.getInvWorld(), m_sdf.getTextureID(), m_computeSDFAlgorithm.getTextureID());
		}

		m_camera.end();
		break;
	}
	case RenderMode::DepthImage:
	default:
		ofBackground(ofColor::black);

		// draw the original image
		ofSetColor(ofColor::white);
		drawFullScreenImage(m_depthImage);

		m_kinect.drawDepth(10, 10, 400, 300);
		m_kinect.draw(420, 10, 400, 300);
		break;
	}

	// Draw GUI
	drawGUI();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	if (key == 'w')
	{
		m_sdf.move(0.5f, 0.0, 0.0);
	}

	if (key == 's')
	{
		m_sdf.move(-0.5f, 0.0, 0.0);
	}

	// Cycle through the render modes
	if (key == OF_KEY_F1)
	{
		int it = static_cast<int>(m_renderMode);
		it++;
		if (it >= static_cast<int>(RenderMode::Max))
		{
			it = 0;
		}

		m_renderMode = static_cast<RenderMode>(it);
	}

	if (key == OF_KEY_F2)
	{
		m_computeSDF = !m_computeSDF;
	}

	if (key == OF_KEY_F4)
	{
		m_sdf.update3dTexture();
	}

	if (key == OF_KEY_F3)
	{
		static int downsample = 0;
		downsample++;
		if (downsample >= 3)
		{
			downsample = 0;
		}

		//m_pointCloud.fillPointCloud(img, m_depthMultipy, downsample);
	}

	if (key == 43)
	{
		m_minDepthGrid += 0.2F;
		//pointCloud.fillPointCloud(img, ++depthMultipy);
	}
	if (key == 45)
	{
		m_minDepthGrid -= 0.2F;
		//pointCloud.fillPointCloud(img, --depthMultipy);
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}
