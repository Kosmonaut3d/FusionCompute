#pragma once

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
	static bool           s_quickDebug;
	static bool           s_drawDepthBackground;
	static GLuint64       s_bilateralBlur_measureComputeTime;
	static bool           s_bilateralBlurCompute;
	static bool           s_bilateralBlurDraw;

	static bool      s_measureTime;
	static GLuint64  s_measureGPUTime2;
	static bool      s_resetView;
	static bool      s_drawHelpers;
	static glm::vec3 s_testPointPos;

	static bool     s_PCL_GPU_compute;
	static bool     s_PCL_GPU_draw;
	static bool     s_PCL_GPU_debugDrawWorld;
	static bool     s_PCL_GPU_debugDrawNormals;
	static GLuint64 s_PCL_GPU_measuredComputeTime;

	static bool s_PCL_CPU_compute;
	static bool s_PCL_CPU_draw;
	static bool s_PCL_CPU_debugDrawNormals;
	static bool s_PCL_CPU_forceUpdate;
	static int  s_PCL_CPU_downscaleExp;
	static int  s_PCL_CPU_downscale;

	static bool     s_sdfCompute;
	static bool     s_sdfComputeColor;
	static GLuint64 s_sdfMeasuredComputeTime;
	static int      s_sdfResolution;
	static float    s_sdfSliceX;
	static bool     s_sdfDrawSlice;
	static bool     s_sdfDrawRaytrace;
	static float    s_sdfWeightTruncation;
	static float    s_sdfTruncation;
	static bool     s_sdfDrawNormals;

	static bool     s_sdfExpand;
	static GLuint64 s_sdfExpandMeasuredComputeTime;

	static bool     s_ICP_CPU_compute;
	static bool     s_ICP_CPU_sum;
	static float    s_ICP_epsilonDist;
	static float    s_ICP_epsilonNor;
	static bool     s_ICP_GPU_compute;
	static GLuint64 s_ICP_correspondenceMeasureTime;
	static GLuint64 s_ICP_GPU_reductionMeasureTime;
	static GLuint64 s_ICP_CPU_solveSystemMeasureTime;
	static GLuint   s_ICP_GPU_correspondenceCount;
	static double   s_ICP_GPU_error;
	static int      s_ICP_iterations;
	static bool     s_ICP_GPU_SDF;
	static bool     s_ICP_GPU_drawDebug;
	static bool     s_ICP_applyTransformation;

  private:
	ofxImGui::Gui m_gui;
};
