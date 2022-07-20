#include "GUIScene.h"

GUIScene::SceneSelection GUIScene::s_sceneSelection = GUIScene::SceneSelection::SDF;

bool   GUIScene::s_isKinectDeliveringData = false;
bool   GUIScene::s_updateKinectData       = true;
ImVec4 GUIScene::s_backgroundColor        = ImVec4(0.2, 0.2, 0.2, 1);
bool   GUIScene::s_quickDebug             = false;
bool   GUIScene::s_drawDepthBackground    = false;

bool      GUIScene::s_measureTime     = true;
GLuint64  GUIScene::s_measureGPUTime2 = 0;
bool      GUIScene::s_resetView       = false;
glm::vec3 GUIScene::s_testPointPos    = glm::vec3(-1, 0, -2);

bool     GUIScene::s_bilateralBlurCompute             = true;
bool     GUIScene::s_bilateralBlurDraw                = true;
GLuint64 GUIScene::s_bilateralBlur_measureComputeTime = 0;

bool GUIScene::s_PCL_GPU_compute          = true;
bool GUIScene::s_PCL_GPU_draw             = false;
bool GUIScene::s_PCL_GPU_debugDrawWorld   = false;
bool GUIScene::s_PCL_GPU_debugDrawNormals = false;

GLuint64 GUIScene::s_PCL_GPU_measuredComputeTime = 0;
bool     GUIScene::s_PCL_CPU_compute             = false;
bool     GUIScene::s_PCL_CPU_forceUpdate         = false;
bool     GUIScene::s_PCL_CPU_draw                = false;
bool     GUIScene::s_PCL_CPU_debugDrawNormals    = false;
int      GUIScene::s_PCL_CPU_downscaleExp        = 3;
int      GUIScene::s_PCL_CPU_downscale           = 8;

bool     GUIScene::s_sdfCompute                   = true;
GLuint64 GUIScene::s_sdfMeasuredComputeTime       = 0;
bool     GUIScene::s_sdfComputeColor              = true;
int      GUIScene::s_sdfResolution                = 512;
bool     GUIScene::s_sdfDrawSlice                 = false;
bool     GUIScene::s_sdfDrawRaytrace              = true;
float    GUIScene::s_sdfSliceX                    = 0.f;
float    GUIScene::s_sdfWeightTruncation          = 200.f;
float    GUIScene::s_sdfTruncation                = .1f;
bool     GUIScene::s_sdfExpand                    = false;
GLuint64 GUIScene::s_sdfExpandMeasuredComputeTime = 0;

// ICP
bool     GUIScene::s_ICP_applyTransformation           = false;
bool     GUIScene::s_ICP_CPU_compute                   = false;
bool     GUIScene::s_ICP_CPU_sum                       = false;
bool     GUIScene::s_ICP_GPU_compute                   = true;
GLuint64 GUIScene::s_ICP_GPU_correspondenceMeasureTime = 0;
GLuint64 GUIScene::s_ICP_GPU_reductionMeasureTime      = 0;
GLuint64 GUIScene::s_ICP_CPU_solveSystemMeasureTime    = 0;
bool     GUIScene::s_ICP_GPU_drawDebug                 = false;
float    GUIScene::s_ICP_epsilonDist                   = .05;
float    GUIScene::s_ICP_epsilonNor                    = .98;
int      GUIScene::s_ICP_GPU_iterations                = 5;
bool     GUIScene::s_ICP_GPU_SDF                       = true;
GLuint   GUIScene::s_ICP_GPU_correspondenceCount       = 0;
double   GUIScene::s_ICP_GPU_error                     = 0;
//---------------------------------------------------
GUIScene::GUIScene()
    : m_gui()
{
}

//---------------------------------------------------
void GUIScene::setup()
{
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	m_gui.setup();
}

//---------------------------------------------------
void GUIScene::update()
{
}

//---------------------------------------------------
void GUIScene::draw(ofEasyCam& camera)
{
	m_gui.begin();

	{
		ImGui::SetWindowPos(ofVec2f(0, 0), ImGuiCond_FirstUseEver);

		// Framerate average for maxTimeDelta
		static float       timeDelta    = 0.0f;
		static double      msAccu       = 0.0f;
		static int         accuCnt      = 0;
		static const float maxTimeDelta = 1.0f;
		static float       msAvg        = 0.0f;

		float ms_current = 1000.0f / ofGetElapsedTimef();

		accuCnt++;
		msAccu += ms_current;
		timeDelta += ImGui::GetIO().DeltaTime;

		if (timeDelta > maxTimeDelta)
		{
			msAvg     = (msAccu / accuCnt);
			accuCnt   = 0;
			msAccu    = 0.0;
			timeDelta = 0.0f;
		}

		// Rolling average
		static double avg_sdfMeasuredComputeTime = 0;
		avg_sdfMeasuredComputeTime               = avg_sdfMeasuredComputeTime * .99 + s_sdfMeasuredComputeTime * .01;

		static double avg_ICP_GPU_correspondenceMeasureTime = 0;
		avg_ICP_GPU_correspondenceMeasureTime =
		    s_ICP_GPU_correspondenceMeasureTime * .99 + s_ICP_GPU_correspondenceMeasureTime * .01;

		ImGui::Text("Application average %.3f (%.1f FPS), %d vertices", msAvg,
		            ofGetFrameRate(), // ImGui::GetIO().Framerate,
		            ImGui::GetIO().MetricsRenderVertices);

		ImGui::TextColored(s_isKinectDeliveringData ? ImColor(50, 200, 50) : ImColor(200, 50, 50),
		                   "Kinect Delivering Data");
		ImGui::Checkbox("Update Kinect Data", &s_updateKinectData);

		ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
		if (ImGui::BeginTabBar("Render Options", tab_bar_flags))
		{
			if (ImGui::BeginTabItem("Performance"))
			{
				const double totalTime = s_PCL_GPU_measuredComputeTime + s_bilateralBlur_measureComputeTime +
				                         avg_sdfMeasuredComputeTime + avg_ICP_GPU_correspondenceMeasureTime +
				                         s_ICP_GPU_reductionMeasureTime + s_ICP_CPU_solveSystemMeasureTime * 1000.0 +
				                         s_sdfExpandMeasuredComputeTime;
				const ImVec2 barSize(200.0f, 0.0f);

				ImGui::ProgressBar(s_bilateralBlur_measureComputeTime / totalTime, barSize);
				ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
				ImGui::Text("Bilateral Blur %f", s_bilateralBlur_measureComputeTime / 1000000.0);

				ImGui::ProgressBar(s_PCL_GPU_measuredComputeTime / totalTime, barSize);
				ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
				ImGui::Text("Point Cloud Computation %f", s_PCL_GPU_measuredComputeTime / 1000000.0);

				ImGui::ProgressBar(avg_sdfMeasuredComputeTime / totalTime, barSize);
				ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
				ImGui::Text("SDF Generation %f", avg_sdfMeasuredComputeTime / 1000000.0);

				// ImGui::ProgressBar(s_sdfExpandMeasuredComputeTime / totalTime, barSize);
				// ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
				// ImGui::Text("SDF Expansion %f", s_sdfExpandMeasuredComputeTime / 1000000.0);

				ImGui::ProgressBar(avg_ICP_GPU_correspondenceMeasureTime / totalTime, barSize);
				ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
				ImGui::Text("ICP correspondences %f", avg_ICP_GPU_correspondenceMeasureTime / 1000000.0);

				ImGui::ProgressBar(s_ICP_GPU_reductionMeasureTime / totalTime, barSize);
				ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
				ImGui::Text("ICP reduction %f", s_ICP_GPU_reductionMeasureTime / 1000000.0);

				ImGui::ProgressBar(s_ICP_CPU_solveSystemMeasureTime * 1000.0 / totalTime, barSize);
				ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
				ImGui::Text("ICP solve linear %f", s_ICP_CPU_solveSystemMeasureTime / 1000.0);

				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("SDF"))
			{
				s_sceneSelection = SceneSelection::SDF;

				static int sdfResExp = static_cast<int>(log2(s_sdfResolution));
				if (ImGui::SliderInt("SDF Resolution", &sdfResExp, 4, 10))
				{
					s_sdfResolution = pow(2, sdfResExp);
				}
				ImGui::SameLine();
				ImGui::Text("eff. %d", s_sdfResolution);

				ImGui::Checkbox("Compute SDF", &s_sdfCompute);
				ImGui::Checkbox("Compute SDF Color", &s_sdfComputeColor);
				ImGui::Text("Compute time %f", s_sdfMeasuredComputeTime / 1000000.0);

				// ImGui::Checkbox("Expand SDF", &s_sdfExpand);
				// ImGui::Text("Expansion time %f", s_sdfExpandMeasuredComputeTime / 1000000.0);

				ImGui::Checkbox("Draw Raytrace", &s_sdfDrawRaytrace);
				ImGui::Text("Draw time %f", s_measureGPUTime2 / 1000000.0);
				ImGui::Checkbox("Draw Slice", &s_sdfDrawSlice);
				ImGui::SliderFloat("SDF Slice X", &s_sdfSliceX, -2.0f, 2.0f);
				ImGui::SliderFloat("SDF Truncation", &s_sdfTruncation, 0, 1);
				ImGui::SliderFloat("SDF Weight Truncation", &s_sdfWeightTruncation, 0, 1000);
				ImGui::Checkbox("Compute Bilateral Blur", &s_bilateralBlurCompute);

				ImGui::Separator();

				const ImVec4 gpuCol = (ImVec4)ImColor(0.5, 170.5f, 0.5f);
				ImGui::PushStyleColor(ImGuiCol_CheckMark, gpuCol);
				ImGui::TextColored(gpuCol, "ICP Compute");
				ImGui::Checkbox("Compute ICP", &s_ICP_GPU_compute);

				ImGui::Text("ICP correspondence time %f", avg_ICP_GPU_correspondenceMeasureTime / 1000000.0);
				ImGui::Text("ICP reduce time %f", s_ICP_GPU_reductionMeasureTime / 1000000.0);

				if (s_ICP_GPU_compute)
				{
					ImGui::Text("correspondences %u", s_ICP_GPU_correspondenceCount);
					ImGui::Text("error %f", s_ICP_GPU_error);
				}

				ImGui::SliderFloat("max dist", &s_ICP_epsilonDist, 0, 1);
				ImGui::SliderFloat("max nor", &s_ICP_epsilonNor, 0, 1);
				ImGui::SliderInt("iterations", &s_ICP_GPU_iterations, 1, 20);
				ImGui::Checkbox("Point to Mesh", &s_ICP_GPU_SDF);

				ImGui::Checkbox("Draw ICP", &s_ICP_GPU_drawDebug);
				ImGui::Checkbox("Apply ICP transformation", &s_ICP_applyTransformation);
				ImGui::PopStyleColor(1);
				if (ImGui::Button("Reset view"))
				{
					s_resetView = true;
				}

				/* ImGui::SliderFloat("Point X", &s_testPointPos.x, -2.0f, 2.0f);
				ImGui::SliderFloat("Point y", &s_testPointPos.y, -2.0f, 2.0f);
				ImGui::SliderFloat("Point Z", &s_testPointPos.z, -4.0f, 0.0f);
				*/

				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Point Cloud"))
			{
				s_sceneSelection = SceneSelection::PointCloud;

				const ImVec4 blurCol = (ImVec4)ImColor(170.5, 170.5f, 0.5f);
				ImGui::PushStyleColor(ImGuiCol_CheckMark, blurCol);
				ImGui::TextColored(blurCol, "Pre process");
				ImGui::Checkbox("Compute Bilateral Blur", &s_bilateralBlurCompute);
				ImGui::PopStyleColor(1);

				ImGui::Separator();

				const ImVec4 gpuCol = (ImVec4)ImColor(0.5, 170.5f, 0.5f);
				ImGui::PushStyleColor(ImGuiCol_CheckMark, gpuCol);
				ImGui::TextColored(gpuCol, "Point Cloud GPU Compute");
				ImGui::Checkbox("Compute PCL", &s_PCL_GPU_compute);
				ImGui::Checkbox("Draw PCL", &s_PCL_GPU_draw);
				ImGui::Checkbox("Draw PCL Model Tex", &s_PCL_GPU_debugDrawWorld);
				ImGui::Checkbox("Draw PCL Normal Tex", &s_PCL_GPU_debugDrawNormals);
				ImGui::PopStyleColor(1);

				ImGui::Separator();

				const ImVec4 cpuCol = (ImVec4)ImColor(0.5, 120.5f, 170.5f);
				ImGui::PushStyleColor(ImGuiCol_CheckMark, cpuCol);
				ImGui::TextColored(cpuCol, "Point Cloud CPU Compute");
				ImGui::Checkbox("Compute PCL CPU", &s_PCL_CPU_compute);
				ImGui::Checkbox("Draw PCL CPU", &s_PCL_CPU_draw);
				ImGui::Checkbox("Draw PCL Normal CPU", &s_PCL_CPU_debugDrawNormals);

				if (ImGui::SliderInt("PCL CPU downscale", &s_PCL_CPU_downscaleExp, 0, 4))
				{
					s_PCL_CPU_downscale   = pow(2, s_PCL_CPU_downscaleExp);
					s_PCL_CPU_forceUpdate = true;
				}
				ImGui::SameLine();
				ImGui::Text("eff. %d", s_PCL_CPU_downscale);

				ImGui::PopStyleColor(1);

				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("ICP"))
			{
				s_sceneSelection = SceneSelection::PointCloud;
				ImGui::Checkbox("Compute ICP CPU", &s_ICP_CPU_compute);
				ImGui::Checkbox("Sum ICP CPU", &s_ICP_CPU_sum);
				if (ImGui::Button("Reset view"))
				{
					s_resetView = true;
				}

				ImGui::SliderFloat("max dist", &s_ICP_epsilonDist, 0, 1);
				ImGui::SliderFloat("max nor", &s_ICP_epsilonNor, 0, 1);

				ImGui::Checkbox("Apply ICP transformation", &s_ICP_applyTransformation);
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Blur"))
			{
				s_sceneSelection = SceneSelection::Blur;
				ImGui::Checkbox("Compute Bilateral Blur", &s_bilateralBlurCompute);
				ImGui::Checkbox("Draw Bilateral Blur", &s_bilateralBlurDraw);
				ImGui::Separator();
				ImGui::Text("Blur time %f", s_bilateralBlur_measureComputeTime / 1000000.0);
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
		ImGui::Dummy(ImVec2(0.0f, 20.0f));
		ImGui::Separator();

		ImGui::ColorEdit3("Background Color", (float*)&s_backgroundColor);
		ImGui::Checkbox("Draw Kinect Depth", &s_drawDepthBackground);
		ImGui::Checkbox("Quick Debug Check", &s_quickDebug);

		// ImGui::ShowDemoWindow();// ::ShowExampleAppDockSpace(true);

		/*if (ImGui::SliderFloat("PCL Size", &m_pclSizeValue, 0.0f, 1000.0f))

		    //this will change the app background color
		    ImGui::ColorEdit3("Background Color", (float*)&m_backgroundColor);

		ImGui::Checkbox("Update Camera", &m_updateKinect);
		ImGui::Checkbox("Draw Slice", &m_drawSlice);
		ImGui::Checkbox("Draw Kinect Depth", &m_drawDepthBackground);
		ImGui::Checkbox("Draw SDF", &m_drawSDF);
		ImGui::Checkbox("Draw PCL", &m_drawPointCloud);
		ImGui::Checkbox("Compute Normals", &m_computeNormalsCPU);
		ImGui::Checkbox("Draw SDF Compute", &m_drawSDFAlgorithm);

		if (ImGui::SliderInt("SDF resolution", &m_sdfResolutionExp, 3, 8))
		{
		    m_sdfResolution = pow(2, m_sdfResolutionExp);
		    //m_sdf.setResolution(m_sdfResolution);
		} ImGui::SameLine();
		ImGui::Text("%d", m_sdfResolution);

		if (ImGui::Button("Compute SDF "))
		{
		    //m_sdf.resetData();
		    m_computeSDF = !m_computeSDF;
		} ImGui::SameLine();

		ImGui::ProgressBar(m_buildProgress);

		if (ImGui::Button("Apply SDF tex"))
		{
		    //m_sdf.update3dTexture();

		}

		if (ImGui::Button("Save Kinect Depth "))
		{
		    if (m_kinect.isConnected())
		    {
		        DataStorageHelper::storeImage("depth.bin", m_depthImage);

		        const auto& pixelsRaw = m_kinect.getRawDepthPixels();
		        const auto* dataRaw = pixelsRaw.getData();
		        DataStorageHelper::storeData("depthRaw.bin", dataRaw, pixelsRaw.getWidth() * pixelsRaw.getHeight());

		        //const auto& pixels = m_kinect.getPixels();
		        //const auto* dataColor = pixels.getData();
		        //DataStorageHelper::storeData("color.bin", dataColor, pixels.getWidth() * pixels.getHeight());
		    }
		}*/
	}

	if (ImGui::GetIO().WantCaptureMouse)
	{
		camera.disableMouseInput();
	}
	else
	{
		camera.enableMouseInput();
	}

	// endcall
	m_gui.end();
}
