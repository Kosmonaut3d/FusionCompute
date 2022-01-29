#include "computeSDF.h"

computeSDF::computeSDF()
{
	if (!m_fsShader.load("resources/SimpleVert.vert", "resources/SimpleFrag.frag"))
	{
		throw std::exception();//"could not load shaders");
	}

	int result;
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &result);

	ofLogNotice() << "max computer work groups(x): " << result;

	int work_grp_cnt[3];

	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_grp_cnt[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_grp_cnt[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_grp_cnt[2]);

	printf("max global (total) work group counts x:%i y:%i z:%i\n",
		work_grp_cnt[0], work_grp_cnt[1], work_grp_cnt[2]);

	int work_grp_size[3];

	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &work_grp_size[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &work_grp_size[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &work_grp_size[2]);

	printf("max local (in one shader) work group sizes x:%i y:%i z:%i\n",
		work_grp_size[0], work_grp_size[1], work_grp_size[2]);

	computeShader.setupShaderFromFile(GL_COMPUTE_SHADER, "resources/computeSDF.comp");
	computeShader.linkProgram();

	outputTexture.allocate(512, 512, GL_RGBA32F);

	setUpOutputTexture();
}

void computeSDF::setUpOutputTexture()
{
	// dimensions of the image
	int tex_w = 512, tex_h = 512;
	
	std::vector<glm::vec4> compute_data(512 * 512);
	std::fill(compute_data.begin(), compute_data.end(), glm::vec4(1, 1, 1, 1));
	glGenTextures(1, &m_texID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_texID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w, tex_h, 0, GL_RGBA, GL_FLOAT, compute_data.data());
	glBindImageTexture(0, m_texID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

	//outputTexture.bindAsImage(0, GL_READ_ONLY);
	//m_mesh.mapTexCoords(-1, -1, 1, 1);
}

void computeSDF::compute()
{
	computeShader.begin();
	computeShader.dispatchCompute(512, 512, 1);

	//std::vector<glm::vec4> compute_data(512*512);
	//glGetTexImage(GL_TEXTURE_2D, m_texID, GL_RGBA, GL_FLOAT, compute_data.data());
	computeShader.end();
}

void computeSDF::draw()
{
	ofMesh quad;

	m_fsShader.begin();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_texID);//outputTexture.getTextureData().textureID);

	GLuint emptyVAO; 
	glGenVertexArrays(1, &emptyVAO); 
	glBindVertexArray(emptyVAO);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	m_fsShader.end();

	//outputTexture.unbind();

	//outputTexture.draw(10+512, 0.0);
}

ofTexture& computeSDF::getTexture()
{
	return outputTexture;
}

unsigned int computeSDF::getTextureID()
{
	return m_texID;
}
