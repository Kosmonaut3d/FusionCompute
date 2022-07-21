#include "slice.h"
#include <scenes/GUIScene.h>

Slice::Slice(ofVec3f position, float size)
    : m_mesh(size, size, 2, 2)
    , m_position(position)
    , m_size(size)
    , m_sliceShader()
{
	m_mesh.set(size, size);
	m_mesh.setPosition(position);
	m_mesh.setScale(1);
	m_mesh.setOrientation(glm::vec3(0, 90, 0));

	if (!m_sliceShader.load("resources/vertShader.vert", "resources/sliceFragShader.frag"))
	{
		throw std::exception(); //"could not load shaders");
	}
}

void Slice::setPos(ofVec3f pos)
{
	m_mesh.setPosition(pos);
}

void Slice::draw(const ofMatrix4x4& sdfInvWorld, unsigned int sdfTextureID, unsigned int computeShaderTexID)
{
	m_mesh.setPosition(m_position + glm::vec3(GUIScene::s_sdfSliceX, 0, 0));
	m_sliceShader.begin();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, sdfTextureID);

	m_sliceShader.setUniformMatrix4f("sdfBaseTransform", sdfInvWorld);
	m_mesh.draw();
	m_sliceShader.end();
}
