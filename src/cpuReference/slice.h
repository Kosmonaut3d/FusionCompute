#pragma once
#include "ofMain.h"

/// <summary>
/// A "slice" that shows a 2D visualization of the SDF
/// </summary>
class Slice
{
  public:
	Slice(ofVec3f position, float size);
	void setPos(ofVec3f pos);
	void draw(const ofMatrix4x4& sdfInvWorld, unsigned int boundTextureID3d, unsigned int boundTextureID);

  private:
	ofPlanePrimitive m_mesh;
	ofVec3f          m_position;
	float            m_size;
	ofShader         m_sliceShader;
};
