#pragma once

#include "ofMain.h"
#include "datastructures/pointCloud.h"
#include "datastructures/sdf.h"
#include "ofxKinect.h"
#include "slice.h"
#include "compute/PointCloudVis.h"
#include "compute/PointCloudCompute.h"
#include "scenes/GUIScene.h"
#include "scenes/PointCloudScene.h"

class ofApp : public ofBaseApp {

public:
	ofApp();

	void setup();
	void update();
	void draw();
	void exit();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);

private:
	void drawKinectPointCloud(ofxKinect& kinect);
	void drawFullScreenImage(ofImage& image);

private:
	GUIScene m_guiScene;
	PointCloudScene m_pointCloudScene;

	ofxKinect m_kinect;
	ofImage m_depthImage; // grayscale depth image
	Slice m_slice;
	ofEasyCam m_camera;

};
