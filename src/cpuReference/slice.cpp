#include "slice.h"
#include <scenes/GUIScene.h>

Slice::Slice(ofVec3f position, float size):
	m_mesh(size, size, 2, 2),
	m_position(position),
	m_size(size),
	m_sliceShader()
{
	m_mesh.set(size, size);
	m_mesh.setPosition(position);
	m_mesh.setScale(1);
	m_mesh.setOrientation(glm::vec3(0, 90, 0));

	if (!m_sliceShader.load("resources/vertShader.vert", "resources/sliceFragShader.frag"))
	{
		throw std::exception();//"could not load shaders");
	}

	//glUniform1i(m_sliceShader.getUniformLocation("tex2D"), 0); // set it manually
	//glUniform1i(m_sliceShader.getUniformLocation("tex3D"), 1); // set it manually
	//m_sliceShader.setUniformTexture("volume_tex", 0 )
}

void Slice::setPos(ofVec3f pos)
{
	m_mesh.setPosition(pos);
}

void Slice::draw(ofMatrix4x4& sdfInvWorld, unsigned int sdfTextureID, unsigned int computeShaderTexID)
{
	m_mesh.setPosition(m_position + glm::vec3(GUIScene::s_sdfSliceX, 0, 0));
	m_sliceShader.begin();

	//m_sliceShader.setUniform1i("tex2D", 0);
	//m_sliceShader.setUniform1i("tex3D", 1);

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, computeShaderTexID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, sdfTextureID);

	m_sliceShader.setUniformMatrix4f("sdfBaseTransform", sdfInvWorld);
	m_mesh.draw();
	m_sliceShader.end();
}
