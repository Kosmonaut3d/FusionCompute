#include "slice.h"
#include "slice.h"

Slice::Slice(ofVec3f position, float size):
	m_mesh(size, size, 2, 2),
	m_position(position),
	m_size(size),
	m_sliceShader()
{

	m_mesh.set(20, 20);
	m_mesh.setPosition(position);
	m_mesh.setScale(1);
	m_mesh.setOrientation(glm::vec3(0, 90, 0));

	if (!m_sliceShader.load("resources/vertShader.vert", "resources/sliceFragShader.frag"))
	{
		throw std::exception();//"could not load shaders");
	}

	//glUniform1i(m_sliceShader.getUniformLocation("volume_tex"), 0); // set it manually
	glUniform1i(m_sliceShader.getUniformLocation("tex2D"),0); // set it manually

	//m_sliceShader.setUniformTexture("volume_tex", 0 )
}

void Slice::setPos(ofVec3f pos)
{
	m_mesh.setPosition(pos);
}

void Slice::draw(ofMatrix4x4& sdfInvWorld, unsigned int boundTextureID3d, unsigned int boundTextureID)
{
	m_sliceShader.begin();

	/*glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, boundTextureID3d);

	glActiveTexture(GL_TEXTURE0 + 1); // Texture unit 1
	glBindTexture(GL_TEXTURE_2D, boundTextureID);
	*/
	glActiveTexture(GL_TEXTURE0); // Texture unit 1
	glBindTexture(GL_TEXTURE_2D, boundTextureID);

	m_sliceShader.setUniformMatrix4f("sdfBaseTransform", sdfInvWorld);
	m_mesh.draw();
	m_sliceShader.end();
}
