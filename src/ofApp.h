#pragma once

#include "ofMain.h"
#include "datastructures/pointCloud.h"
#include "datastructures/sdf.h"
#include "ofxKinect.h"
#include "ofxImGui.h"
#include "slice.h"
#include "algorithms/computeSDF.h"
#include "compute/PointCloudComp.h"

class ofApp : public ofBaseApp {
	enum class RenderMode
	{
		DepthImage = 0,
		SDF = 1,
		Max = 2,
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
	ofTexture m_depthRawTexture;
	RenderMode m_renderMode;
	PointCloud m_pointCloud;
	PointCloudComp m_pointCloudComp;
	Slice m_slice;
	ofEasyCam m_camera;
	SignedDistanceField m_sdf;
	computeSDF m_computeSDFAlgorithm;
	float m_depthMultipy;
	float m_minDepthGrid;
	bool m_computeSDF;
	bool m_drawSlice;
	bool m_drawSDF;
	bool m_drawPointCloud;
	bool m_computeNormalsCPU;
	bool m_drawDepthBackground;
	bool m_drawSDFAlgorithm;
	float m_buildProgress;
	int m_sdfResolutionExp;
	int m_sdfResolution;
	bool m_updateKinect;

	ImVec4 m_backgroundColor;
	float m_floatValue;
};
