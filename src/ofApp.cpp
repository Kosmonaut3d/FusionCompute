#include "ofApp.h"

ofApp::ofApp() : ofBaseApp(),
	sdf(16, glm::vec3(-10,-10,-20), 20),
	depthMultipy{2.0f},
	minDepthGrid{2.0f},
	renderMode{RenderMode::PointCloud},
	computeSDF{false}
{

}

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetVerticalSync(false);
	ofSetFrameRate(0);
	img.load("resources/depth.png");
	ofSetLineWidth(2);

	ofBackground(70, 70, 70);
	ofEnableDepthTest();

	mainCamera.setPosition(glm::vec3(0, 0, 0));
	mainCamera.setTarget(glm::vec3(0, 0, -1)); // look forward
	mainCamera.setDistance(10);
	mainCamera.setNearClip(10);
	mainCamera.setFarClip(10000);
	mainCamera.setTranslationKey(32); // space

	pointCloud.fillPointCloud(img, depthMultipy,0);

	//sdf.insertPoint(glm::vec3(0,0,-10), glm::vec3(0,0,0), 0.9f);
}

//--------------------------------------------------------------
void ofApp::update(){
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
					break;
				}

				sdf.insertPoint(glm::vec3(pointCloud.getPoints()[k]), glm::vec3(0, 0, 0), 0.95f);
				auto color = mesh.getColor(k);
				mesh.setColor(k, color * ofColor::red);
			}
			i += batchsize;
		}
	}
}

//--------------------------------------------------------------
void ofApp::draw()
{
	switch (renderMode)
	{
		case RenderMode::PointCloud:
			mainCamera.begin();
			ofEnableDepthTest();
			ofBackground(40,40,40);

			ofPushStyle();

			//give a saturation and lightness
			ofSetColor(255, 100, 100);

			ofDrawGrid(100.0f);

			ofPopStyle();

			sdf.drawOutline();
			sdf.drawGrid(minDepthGrid);

			pointCloud.draw();
			mainCamera.end();
			break;
		case RenderMode::DepthImage:
		default:
			ofBackground(ofColor::black);

			// draw the original image
			ofSetColor(ofColor::white);
			img.draw(0, 0, img.getWidth() * 2.0f, img.getHeight()*2.0f);
			break;
	}

	ofDrawBitmapString("Press F1 to cycle render modes", 20, 30);
	ofDrawBitmapString("Press F2 to compute the SDF by CPU", 20, 50);
	ofDrawBitmapString("Press +/- to change the depth " + std::to_string(minDepthGrid), 20, 70);
	ofDrawBitmapString(ofToString(ofGetFrameRate()) + "fps", 10, 15);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
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
