#include "fullScreenQuadRender.h"

FullScreenQuadRender::FullScreenQuadRender()
{
	if (!m_shader.isLoaded())
	{
		if (!m_shader.load("resources/fullScreenQuad.vert", "resources/fullScreenQuad.frag"))
		{
			throw std::exception(); //"could not load shaders");
		}
	}
}

void FullScreenQuadRender::draw(ofTexture tex)
{
	auto const texData = tex.getTextureData();
	draw(texData.textureID, texData.textureTarget);
}

void FullScreenQuadRender::draw(unsigned int texID, GLenum textureTarget = GL_TEXTURE_2D)
{
	m_shader.begin();

	glUniform1i(m_shader.getUniformLocation("t"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(textureTarget, texID); // outputTexture.getTextureData().textureID);

	GLuint emptyVAO;
	glGenVertexArrays(1, &emptyVAO);
	glBindVertexArray(emptyVAO);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(textureTarget, 0);

	m_shader.end();
}

FullScreenQuadRender &FullScreenQuadRender::get()
{
	static FullScreenQuadRender instance;
	return instance;
}
