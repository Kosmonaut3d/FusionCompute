#include "GUIScene.h"

bool GUIScene::s_isKinectDeliveringData = false;
bool GUIScene::s_updateKinectData = true;
ImVec4 GUIScene::s_backgroundColor = ImVec4(0.2, 0.2, 0.2, 1);
bool GUIScene::s_computePointCloud = true;
bool GUIScene::s_drawPointCloud = true;
bool GUIScene::s_drawPointCloudTex = false;
bool GUIScene::s_drawDepthBackground = false;

//---------------------------------------------------
GUIScene::GUIScene() : m_gui()
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

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS), %d vertices", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate, ImGui::GetIO().MetricsRenderVertices);

		ImGui::MenuItem("Kinect Connected", NULL, s_isKinectDeliveringData);
		ImGui::Checkbox("Update Kinect Data", &s_updateKinectData);

		ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
		if (ImGui::BeginTabBar("Render Options", tab_bar_flags))
		{
			if (ImGui::BeginTabItem("Point Cloud"))
			{
				ImGui::Checkbox("Compute PCL", &s_computePointCloud);
				ImGui::Checkbox("Draw PCL", &s_drawPointCloud);
				ImGui::Checkbox("Draw PCL World Tex", &s_drawPointCloudTex);
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
		ImGui::Separator();

		ImGui::ColorEdit3("Background Color", (float*)&s_backgroundColor);
		ImGui::Checkbox("Draw Kinect Depth", &s_drawDepthBackground);

		//ImGui::ShowDemoWindow();// ::ShowExampleAppDockSpace(true);
		/*if (ImGui::SliderFloat("Float", &m_floatValue, -10.0f, 10.0f))
		{
			m_slice.setPos(glm::vec3(m_floatValue, 0, -10));
		};*/

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



