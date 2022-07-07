#pragma once

#include "SceneImpl.h"
#include "ofMain.h"
#include "ofxImGui.h"

class GUIScene //: public SceneImpl
{
  public:
	enum SceneSelection
	{
		PointCloud,
		Blur,
		SDF
	};

	GUIScene();
	void setup();
	void update();
	void draw(ofEasyCam& camera);

	static SceneSelection s_sceneSelection;
	static bool           s_isKinectDeliveringData;
	static bool           s_updateKinectData;
	static ImVec4         s_backgroundColor;
	static bool           s_computePointCloud;
	static bool           s_drawPointCloud;
	static bool           s_drawPointCloudTex;
	static bool           s_drawPointCloudNorm;
	static bool           s_computePointCloudCPU;
	static bool           s_drawPointCloudCPU;
	static bool           s_drawPointCloudNormCPU;
	static bool           s_pointCloudCPUForceUpdate;
	static int            s_pointCloudDownscaleExp;
	static int            s_pointCloudDownscale;
	static bool           s_ICP_CPU_compute;
	static bool           s_ICP_CPU_sum;
	static float          s_ICP_epsilonDist;
	static float          s_ICP_epsilonNor;
	static GLuint         s_ICP_GPU_correspondenceCount;
	static int            s_ICP_GPU_iterations;
	static bool           s_ICP_GPU_SDF;
	static bool           s_ICP_applyTransformation;
	static bool           s_quickDebug;
	static bool           s_drawDepthBackground;
	static bool           s_bilateralBlurCompute;
	static bool           s_bilateralBlurDraw;
	static GLuint64       s_measureGPUTime;
	static GLuint64       s_measureGPUTime2;
	static GLuint64       s_measureGPUTime_reduction;
	static bool           s_sdfCompute;
	static int            s_sdfResolution;
	static float          s_sdfSliceX;
	static bool           s_sdfDrawSlice;
	static bool           s_sdfDrawRaytrace;
	static float          s_sdfWeightTruncation;
	static float          s_sdfTruncation;
	static bool           s_ICP_GPU_compute;
	static bool           s_drawICPGPU;
	static bool           s_resetView;
	static glm::vec3      s_testPointPos;

  private:
	ofxImGui::Gui m_gui;
};
