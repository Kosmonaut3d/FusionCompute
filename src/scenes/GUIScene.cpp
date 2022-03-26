#include "GUIScene.h"

GUIScene::SceneSelection GUIScene::s_sceneSelection = GUIScene::SceneSelection::Blur;

bool      GUIScene::s_isKinectDeliveringData   = false;
bool      GUIScene::s_updateKinectData         = true;
ImVec4    GUIScene::s_backgroundColor          = ImVec4(0.2, 0.2, 0.2, 1);
bool      GUIScene::s_computePointCloud        = true;
bool      GUIScene::s_drawPointCloud           = true;
bool      GUIScene::s_drawPointCloudTex        = false;
bool      GUIScene::s_drawPointCloudNorm       = false;
bool      GUIScene::s_computePointCloudCPU     = false;
bool      GUIScene::s_pointCloudCPUForceUpdate = false;
bool      GUIScene::s_drawPointCloudCPU        = false;
bool      GUIScene::s_drawPointCloudNormCPU    = false;
int       GUIScene::s_pointCloudDownscaleExp   = 3;
int       GUIScene::s_pointCloudDownscale      = 8;
bool      GUIScene::s_computeICPCPU            = false;
float     GUIScene::s_ICP_epsilonDist          = .2;
float     GUIScene::s_ICP_epsilonNor           = .8;
bool      GUIScene::s_quickDebug               = false;
bool      GUIScene::s_drawDepthBackground      = false;
bool      GUIScene::s_bilateralBlurCompute     = true;
bool      GUIScene::s_bilateralBlurDraw        = true;
GLuint64  GUIScene::s_measureGPUTime           = 0;
GLuint64  GUIScene::s_measureGPUTime2          = 0;
bool      GUIScene::s_sdfCompute               = false;
int       GUIScene::s_sdfResolution            = 64;
bool      GUIScene::s_sdfDrawSlice             = false;
bool      GUIScene::s_sdfDrawRaytrace          = true;
float     GUIScene::s_sdfSliceX                = 0.f;
float     GUIScene::s_sdfWeightTruncation      = 20.f;
float     GUIScene::s_sdfTruncation            = 3.f;
bool      GUIScene::s_computeICPGPU            = true;
bool      GUIScene::s_drawICPGPU               = true;
bool      GUIScene::s_resetView                = false;
glm::vec3 GUIScene::s_testPointPos             = glm::vec3(-1, 0, -2);

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

		float ms_current = 1000.0f / ImGui::GetIO().Framerate;

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

		ImGui::Text("Application average %.3f (%.1f FPS), %d vertices", msAvg, ImGui::GetIO().Framerate,
		            ImGui::GetIO().MetricsRenderVertices);

		ImGui::TextColored(s_isKinectDeliveringData ? ImColor(50, 200, 50) : ImColor(200, 50, 50),
		                   "Kinect Delivering Data");
		ImGui::Checkbox("Update Kinect Data", &s_updateKinectData);

		ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
		if (ImGui::BeginTabBar("Render Options", tab_bar_flags))
		{
			if (ImGui::BeginTabItem("SDF"))
			{
				s_sceneSelection = SceneSelection::SDF;

				static int sdfResExp = static_cast<int>(log2(s_sdfResolution));
				if (ImGui::SliderInt("SDF Resolution", &sdfResExp, 4, 9))
				{
					s_sdfResolution = pow(2, sdfResExp);
				}
				ImGui::SameLine();
				ImGui::Text("eff. %d", s_sdfResolution);

				ImGui::Checkbox("Compute SDF", &s_sdfCompute);
				ImGui::Text("Compute time %f", s_measureGPUTime / 1000000.0);
				ImGui::Checkbox("Draw Raytrace", &s_sdfDrawRaytrace);
				ImGui::Text("Draw time %f", s_measureGPUTime2 / 1000000.0);
				ImGui::Checkbox("Draw Slice", &s_sdfDrawSlice);
				ImGui::SliderFloat("SDF Slice X", &s_sdfSliceX, -2.0f, 2.0f);
				ImGui::SliderFloat("SDF Truncation", &s_sdfTruncation, 0, 10);
				ImGui::SliderFloat("SDF Weight Truncation", &s_sdfWeightTruncation, 0, 1000);
				ImGui::Checkbox("Compute Bilateral Blur", &s_bilateralBlurCompute);

				ImGui::Separator();

				const ImVec4 gpuCol = (ImVec4)ImColor(0.5, 170.5f, 0.5f);
				ImGui::PushStyleColor(ImGuiCol_CheckMark, gpuCol);
				ImGui::TextColored(gpuCol, "ICP Compute");
				ImGui::Checkbox("Compute ICP", &s_computeICPGPU);
				ImGui::Checkbox("Draw ICP", &s_drawICPGPU);
				ImGui::PopStyleColor(1);

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
				ImGui::Checkbox("Compute PCL", &s_computePointCloud);
				ImGui::Checkbox("Draw PCL", &s_drawPointCloud);
				ImGui::Checkbox("Draw PCL Model Tex", &s_drawPointCloudTex);
				ImGui::Checkbox("Draw PCL Normal Tex", &s_drawPointCloudNorm);
				ImGui::PopStyleColor(1);

				ImGui::Separator();

				const ImVec4 cpuCol = (ImVec4)ImColor(0.5, 120.5f, 170.5f);
				ImGui::PushStyleColor(ImGuiCol_CheckMark, cpuCol);
				ImGui::TextColored(cpuCol, "Point Cloud CPU Compute");
				ImGui::Checkbox("Compute PCL CPU", &s_computePointCloudCPU);
				ImGui::Checkbox("Draw PCL CPU", &s_drawPointCloudCPU);
				ImGui::Checkbox("Draw PCL Normal CPU", &s_drawPointCloudNormCPU);

				if (ImGui::SliderInt("PCL CPU downscale", &s_pointCloudDownscaleExp, 0, 4))
				{
					s_pointCloudDownscale      = pow(2, s_pointCloudDownscaleExp);
					s_pointCloudCPUForceUpdate = true;
				}
				ImGui::SameLine();
				ImGui::Text("eff. %d", s_pointCloudDownscale);

				ImGui::PopStyleColor(1);

				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("ICP"))
			{
				s_sceneSelection = SceneSelection::PointCloud;
				ImGui::Checkbox("Compute ICP CPU", &s_computeICPCPU);
				if (ImGui::Button("Reset view"))
				{
					s_resetView = true;
				}

				ImGui::SliderFloat("max dist", &s_ICP_epsilonDist, 0, 4);
				ImGui::SliderFloat("max nor", &s_ICP_epsilonNor, 0, 1);
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Blur"))
			{
				s_sceneSelection = SceneSelection::Blur;
				ImGui::Checkbox("Compute Bilateral Blur", &s_bilateralBlurCompute);
				ImGui::Checkbox("Draw Bilateral Blur", &s_bilateralBlurDraw);
				ImGui::Separator();
				ImGui::Text("Blur time %f", s_measureGPUTime / 1000000.0);
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
