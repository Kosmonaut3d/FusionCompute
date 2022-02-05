#include "PointCloudComp.h"

PointCloudComp::PointCloudComp() : m_shader{}
{
	if (!m_shader.load("resources/pointCloud.vert", "resources/pointCloud.frag"))
	{
		throw std::exception();//"could not load shaders");
	}
}

/// <summary>
/// 
/// </summary>
void PointCloudComp::draw(unsigned int pointCloudTexId, unsigned int rgbTexId, bool drawNormals, glm::mat4x4& mvpMat, float factor)
{
	m_shader.begin();

	m_shader.setUniformMatrix4f("modelViewProjectionMatrix", mvpMat);

	m_shader.setUniform1f("mixFactor", factor);

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, pointCloudTexId);//outputTexture.getTextureData().textureID);

	glBindImageTexture(0, pointCloudTexId, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	//glBindImageTexture(1, rgbTexId, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

	GLuint emptyVAO;
	glGenVertexArrays(1, &emptyVAO);
	glBindVertexArray(emptyVAO);
	glPointSize(4.0f);
	glDrawArrays(GL_POINTS, 0, 640*480);

	m_shader.end();
}