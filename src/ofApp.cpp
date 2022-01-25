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
	img.load("resources/depth.png");
	ofSetLineWidth(2);

	ofBackground(70, 70, 70);
	ofEnableDepthTest();

	m_camera.setPosition(glm::vec3(0, 0, 0));
	m_camera.setTarget(glm::vec3(0, 0, -1)); // look forward
	m_camera.setDistance(10);
	m_camera.setNearClip(10);
	m_camera.setFarClip(1000);
	m_camera.setTranslationKey(32); // space

	pointCloud.fillPointCloud(img, depthMultipy, 3);

	glEnable(GL_TEXTURE_3D);
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
			m_camera.begin();
			ofEnableDepthTest();
			ofBackground(40,40,40);

			ofPushStyle();

			//give a saturation and lightness
			ofSetColor(255, 100, 100);

			//ofDrawGrid(100.0f);

			ofPopStyle();

			sdf.drawOutline();
            //sdf.drawGrid(minDepthGrid);

			pointCloud.draw();
			m_camera.end();
			break;
		case RenderMode::DepthImage:
		default:
			ofBackground(ofColor::black);

			// draw the original image
			ofSetColor(ofColor::white);
            int width = ofGetViewportWidth();
            int height = ofGetViewportHeight();

            float aspectImage = img.getWidth()/static_cast<float>(img.getHeight());
            float aspectView = width / static_cast<float>(height);

            if(aspectView >= aspectImage)
            {
                width = static_cast<int>(aspectImage * height);
            }
            else
            {
                height = static_cast<int>(width / aspectImage);
            }

            img.draw(0, 0, width, height);
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
