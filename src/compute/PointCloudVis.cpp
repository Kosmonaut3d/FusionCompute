#include "PointCloudVis.h"

PointCloudVis::PointCloudVis()
    : m_shader{}
{
	if (!m_shader.load("shaders/pointCloud.vert", "shaders/pointCloud.frag"))
	{
		throw std::exception(); //"could not load shaders");
	}
}

void PointCloudVis::draw(unsigned int pointCloudTexId, unsigned int rgbTexId, bool drawNormals,
                         const glm::mat4x4& mvpMat)
{
	m_shader.begin();

	m_shader.setUniformMatrix4f("modelViewProjectionMatrix", mvpMat);

	glUniform1i(m_shader.getUniformLocation("worldTex"), 0);
	glUniform1i(m_shader.getUniformLocation("colorTex"), 1);

	glBindImageTexture(0, pointCloudTexId, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, rgbTexId);

	GLuint emptyVAO;
	glGenVertexArrays(1, &emptyVAO);
	glBindVertexArray(emptyVAO);
	glPointSize(1.0f);
	glDrawArrays(GL_POINTS, 0, 640 * 480);

	m_shader.end();
}
