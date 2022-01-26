#pragma once

#include "ofMain.h"
#include "datastructures/pointCloud.h"
#include "datastructures/sdf.h"
#include "ofxKinect.h"
#include "ofxImGui.h"

class ofApp : public ofBaseApp {
	enum class RenderMode
	{
		DepthImage = 0,
		PointCloud = 1,
		SDF = 2,
		Max = 3,
	};

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
	void drawGUI();

private:
	ofxImGui::Gui m_gui;
	ofxKinect m_kinect;
	ofImage m_depthImage; // grayscale depth image
	RenderMode m_renderMode;
	PointCloud m_pointCloud;
	ofEasyCam m_camera;
	SignedDistanceField m_sdf;
	float m_depthMultipy;
	float m_minDepthGrid;
	bool m_computeSDF;
	float m_buildProgress;

	ImVec4 m_backgroundColor;
	float m_floatValue;
};
