#pragma once
#include <ofMain.h>
#include <ofxKinect.h>

/// <summary>
///	This class deploys shaders that render point cloud data efficiently.
/// </summary>
class PointCloudVis
{
  public:
	PointCloudVis();

	void draw(unsigned int pointCloudTexId, unsigned int rgbTexId, bool drawNormals, const glm::mat4x4& mvpMat);

  private:
	ofShader m_shader;
};
