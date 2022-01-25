#include "ofApp.h"

ofApp::ofApp() : ofBaseApp(),
    sdf(128, glm::vec3(-10,-10,-20), 20, 2),
	depthMultipy{2.0f},
	minDepthGrid{2.0f},
	renderMode{RenderMode::SDF},
	computeSDF{false}
{
}

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetVerticalSync(false);
	ofSetFrameRate(0);
    ofDisableSmoothing();
	ofSetLineWidth(2);

	ofBackground(70, 70, 70);
	ofEnableDepthTest();

	m_camera.setPosition(glm::vec3(0, 0, 0));
	m_camera.setTarget(glm::vec3(0, 0, -1)); // look forward
	m_camera.setDistance(10);
	m_camera.setNearClip(1);
	m_camera.setFarClip(1000);
	m_camera.setTranslationKey(32); // space


	glEnable(GL_TEXTURE_3D);

	// enable depth->video image calibration
	kinect.setRegistration(true);

	kinect.init();
	//kinect.init(true); // shows infrared instead of RGB video image
	//kinect.init(false, false); // disable video image (faster fps)

	kinect.open();		// opens first available kinect
	//kinect.open(1);	// open a kinect by id, starting with 0 (sorted by serial # lexicographically))
	//kinect.open("A00362A08602047A");	// open a kinect using it's unique serial #

	// print the intrinsic IR sensor values
	if (kinect.isConnected()) {
		ofLogNotice() << "sensor-emitter dist: " << kinect.getSensorEmitterDistance() << "cm";
		ofLogNotice() << "sensor-camera dist:  " << kinect.getSensorCameraDistance() << "cm";
		ofLogNotice() << "zero plane pixel size: " << kinect.getZeroPlanePixelSize() << "mm";
		ofLogNotice() << "zero plane dist: " << kinect.getZeroPlaneDistance() << "mm";
	}

	depthImage.allocate(kinect.width, kinect.height, ofImageType::OF_IMAGE_GRAYSCALE);
	//img.load("resources/depth.png");
	//pointCloud.fillPointCloud(img, depthMultipy, 3);
}

void ofApp::exit()
{
	kinect.close();
}

void ofApp::drawPointCloud() {
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

//--------------------------------------------------------------
void ofApp::update(){

	kinect.update();

	if (kinect.isFrameNew())
	{
		depthImage.setFromPixels(kinect.getDepthPixels());
		depthImage.update();
		pointCloud.fillPointCloud(kinect, 2);
	}

	static int i = 0;
	if (computeSDF)
	{
		const int batchsize = 2000;
		auto& mesh = pointCloud.getMesh();
		const ofColor finishColor = ofColor::red;

		if (pointCloud.getSize() > 0)
		{
			for (int j = 0; j < batchsize; j++)
			{
				int k = j + i;

				if (k >= pointCloud.getSize())
				{
					i = 0;
					computeSDF = false;

					sdf.storeData();

					break;
				}

				sdf.insertPoint(glm::vec3(pointCloud.getPoints()[k]), glm::vec3(0, 0, 0), 0.75f, 0.1f);
				auto color = mesh.getColor(k);
				mesh.setColor(k, color * ofColor::red);
			}
			i += batchsize;
			m_buildProgress = 1.f* i / pointCloud.getSize();
		}
	}
}

//--------------------------------------------------------------
void ofApp::draw()
{
	switch (renderMode)
	{
		case RenderMode::SDF:
			m_camera.begin();
			ofEnableDepthTest();
			ofBackground(40, 40, 40);

			ofPushStyle();

			//give a saturation and lightness
			ofSetColor(255, 100, 100);

			//ofDrawGrid(100.0f);

			ofPopStyle();

			sdf.drawOutline();
			sdf.drawRaymarch(m_camera);

			m_camera.end();
			break;
		case RenderMode::PointCloud:
		{

			ofSetColor(ofColor::white*0.2);
			drawFullScreenImage(depthImage);

			m_camera.begin();
			ofEnableDepthTest();
			//ofBackground(40, 40, 40);

			ofPushStyle();

			//give a saturation and lightness
			ofSetColor(255, 100, 100);

			//ofDrawGrid(100.0f);

			ofPopStyle();

			sdf.drawOutline();
			//sdf.drawGrid(minDepthGrid);

			pointCloud.draw();
			//drawPointCloud();
			m_camera.end();
			break;
		}
		case RenderMode::DepthImage:
		default:
			ofBackground(ofColor::black);

			// draw the original image
			ofSetColor(ofColor::white);
			drawFullScreenImage(depthImage);

			kinect.drawDepth(10, 10, 400, 300);
			kinect.draw(420, 10, 400, 300);
			break;
	}

	ofDrawBitmapString("Press F1 to cycle render modes", 20, 30);
	ofDrawBitmapString("Press F2 to compute the SDF by CPU, "+std::to_string(m_buildProgress)+"%", 20, 50);
	ofDrawBitmapString("Press +/- to change the depth " + std::to_string(minDepthGrid), 20, 70);
	ofDrawBitmapString(ofToString(ofGetFrameRate()) + "fps", 10, 15);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	if (key == 'w')
	{
		sdf.move(0.5f, 0.0, 0.0);
	}

	if (key == 's')
	{
		sdf.move(-0.5f, 0.0, 0.0);
	}

	// Cycle through the render modes
	if (key == OF_KEY_F1)
	{
		int it = static_cast<int>(renderMode);
		it++;
		if (it >= static_cast<int>(RenderMode::Max))
		{
			it = 0;
		}

		renderMode = static_cast<RenderMode>(it);
	}

	if (key == OF_KEY_F2)
	{
		computeSDF = !computeSDF;
	}

	if (key == OF_KEY_F4)
	{
		sdf.update3dTexture();
	}

	if (key == OF_KEY_F3)
	{
		static int downsample = 0;
		downsample++;
		if (downsample >= 3)
		{
			downsample = 0;
		}

		pointCloud.fillPointCloud(img, depthMultipy, downsample);
	}

	if (key == 43)
	{
		minDepthGrid+=0.2F;
		//pointCloud.fillPointCloud(img, ++depthMultipy);
	}
	if (key == 45)
	{
		minDepthGrid-=0.2F;
		//pointCloud.fillPointCloud(img, --depthMultipy);
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
