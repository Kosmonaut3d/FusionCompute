#include "PreprocessDepthScene.h"

#include "helper/fullScreenQuadRender.h"

//---------------------------------------------------
PreprocessDepthScene::PreprocessDepthScene()
    : m_bilateralBlurComp{}
    , m_texDepthRaw{}
{
	
}

//----------------------------------------------------------------------------------------------------------
void PreprocessDepthScene::setup(ofxKinect& kinect)
{
	m_texDepthRaw.allocate(kinect.getRawDepthPixels());
	update(true, kinect);
}

//----------------------------------------------------------------------------------------------------------
void PreprocessDepthScene::update(bool kinectUpdate, ofxKinect& kinect)
{
	if (kinectUpdate)
	{
		m_texDepthRaw.loadData(kinect.getRawDepthPixels());
	}

	// Just for this scene we do it every frame :)
	m_bilateralBlurComp.compute(m_texDepthRaw, GUIScene::s_bilateralBlurCompute);
}

//----------------------------------------------------------------------------------------------------------
void PreprocessDepthScene::draw()
{
	if (GUIScene::s_bilateralBlurDraw)
	{
		FullScreenQuadRender::get().draw(m_bilateralBlurComp.getTextureID(), GL_TEXTURE_2D);
	}
}

