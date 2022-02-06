#pragma once

#include "ofMain.h"
#include "SceneImpl.h"
#include "ofxImGui.h"

class GUIScene //: public SceneImpl
{

public:
	GUIScene();
	void setup() ;
	void update() ;
	void draw(ofEasyCam& camera) ;

	static bool s_isKinectDeliveringData;
	static bool s_updateKinectData;
	static ImVec4 s_backgroundColor;
	static bool s_computePointCloud;
	static bool s_drawPointCloud;
	static bool s_drawPointCloudTex;
	static bool s_drawPointCloudNorm;
	static bool s_quickDebug;
	static bool s_drawDepthBackground;
private:
	ofxImGui::Gui m_gui;
};