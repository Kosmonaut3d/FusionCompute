#include "fullScreenQuadRender.h"

FullScreenQuadRender::FullScreenQuadRender()
{
	if (!m_shader.isLoaded())
	{
		if (!m_shader.load("resources/fullScreenQuad.vert", "resources/fullScreenQuad.frag"))
		{
			throw std::exception();//"could not load shaders");
		}
	}

	glGenTextures(1, &testID);
	glBindTexture(GL_TEXTURE_2D, testID);
	// set the texture wrapping/filtering options (on the currently bound texture object)
	/*glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);*/

	unsigned char data[] = { 255, 255, 0, 255 };

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glBindTexture(GL_TEXTURE_2D, 0);
	//glGenerateMipmap(GL_TEXTURE_2D);
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
	glBindTexture(textureTarget, texID);//outputTexture.getTextureData().textureID);

	GLuint emptyVAO;
	glGenVertexArrays(1, &emptyVAO);
	glBindVertexArray(emptyVAO);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(textureTarget, 0);

	m_shader.end();
}

FullScreenQuadRender& FullScreenQuadRender::get()
{
	static FullScreenQuadRender instance;
	return instance;
}
