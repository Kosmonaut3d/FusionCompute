#include "sdf.h"

SignedDistanceField::SignedDistanceField(int resolution, glm::vec3 origin, float scale):
	m_boxMesh(scale, scale, scale)
{
	this->m_resolution = resolution;
	m_resolutionSq = resolution * resolution;
	this->m_origin = origin;
	this->m_scale = scale;
	m_world = ofMatrix4x4::newScaleMatrix(scale, scale, scale) * ofMatrix4x4::newTranslationMatrix(origin);
	m_distanceField = std::vector<float>();
	m_distanceField.resize(resolution * resolution * resolution); 
	std::fill(m_distanceField.begin(), m_distanceField.end(), 10000000.0F);

	if (!m_raymarchShader.load("resources/vertShader.vert", "resources/fragShader.frag"))
	{
		throw std::exception("could not load shaders");
	}

	//m_boxMesh.setPosition(m_origin*2); 
	create3dTexture(m_resolution);
}

void SignedDistanceField::drawOutline()
{
	ofPushStyle();
	ofGetCurrentRenderer()->setFillMode(ofFillFlag::OF_OUTLINE);
	ofSetColor(255, 0, 0);
	//ofDrawBox(m_origin, 1,1,1);
	ofVec3f vec = ofVec3f(0, 0, 0);
	vec = ofVec3f(m_origin) * m_world.getInverse();
	ofDrawBox(vec, 1, 1, 1);

	ofSetColor(200, 100, 200);
	float scalehalf = m_scale / 2;
	ofDrawBox(m_origin+glm::vec3(scalehalf, scalehalf, scalehalf), m_scale, m_scale, m_scale);
	ofPopStyle();
}

void SignedDistanceField::drawRaymarch(ofCamera& camera)
{
	ofPushStyle();
	m_raymarchShader.begin();
	glPolygonMode(GL_BACK, GL_LINE);
	float scalehalf = m_scale / 2;
	glCullFace(GL_CCW);

	m_raymarchShader.setUniform3f("cameraWorld", camera.getPosition());
	m_raymarchShader.setUniformMatrix4f("sdfBaseTransform", m_world.getInverse());
	m_raymarchShader.setUniform1f("sdfResolution", m_resolution);

	ofDrawBox(m_origin + glm::vec3(scalehalf, scalehalf, scalehalf), m_scale, m_scale, m_scale);
	m_raymarchShader.end();
	ofPopStyle();
}


void SignedDistanceField::drawGrid(float minDistance)
{
	ofPushStyle();
	auto stepsize = m_scale / m_resolution;
	auto halfstep = stepsize / 2;

	ofGetCurrentRenderer()->setFillMode(ofFillFlag::OF_OUTLINE);

	ofSetColor(255, 255, 255);

	for (int x = 0; x < m_resolution; x++)
	{
		for (int y = 0; y < m_resolution; y++)
		{
			for (int z = 0; z < m_resolution; z++)
			{
				auto sdfValue = m_distanceField[getIndexFromXYZ(x, y, z)];

				if (sdfValue > minDistance)
				{
					continue;
				}

				int value = (1.0f - (sdfValue / minDistance))*255;
				ofSetColor(value, value, value);

				auto localMiddleOfBox = m_origin + glm::vec3(stepsize * x + halfstep, stepsize * y + halfstep, stepsize * z + halfstep);
				ofDrawBox(localMiddleOfBox, stepsize, stepsize, stepsize);
			}
		}
	}
	ofPopStyle();
}

void SignedDistanceField::move(float x, float y, float z)
{
	//auto translation = ofMatrix4x4::newTranslationMatrix(glm::vec4(x, y, z, 0));
	//m_origin.mul;
	m_origin += glm::vec3(x, y, z);
}

glm::vec3 SignedDistanceField::getXYZFromIndex(int index)
{
	int z = index / m_resolutionSq;
	index = index % m_resolutionSq;
	int y = index / m_resolution;
	index = index % m_resolution;
	int x = index;
	return glm::vec3(x, y, z);
}

int SignedDistanceField::getIndexFromXYZ(int x, int y, int z)
{
	return z*m_resolutionSq+y*m_resolution+x;
}

void SignedDistanceField::insertPoint(glm::vec3 point, glm::vec3 cameraOrigin, float minDotValueForBehind)
{
	if (point == glm::vec3(0, 0, 0))
	{
		return;
	}
	auto stepsize = m_scale / m_resolution;
	auto halfstep = stepsize / 2;
	for (int x = 0; x < m_resolution; x++)
	{
		for (int y = 0; y < m_resolution; y++)
		{
			for (int z = 0; z < m_resolution; z++)
			{
				auto index = getIndexFromXYZ(x, y, z);
				auto sdfValue = m_distanceField[index];
				auto localMiddleOfBox = m_origin + glm::vec3(stepsize * x + halfstep, stepsize * y + halfstep, stepsize * z + halfstep);

				auto gridVector = localMiddleOfBox - point;

				auto newDistance = glm::length(gridVector);

				if (newDistance < abs(sdfValue))
				{
					auto cameraVector = point - cameraOrigin;

					// Check if we are "behind" our target point
					if (glm::dot(glm::normalize(gridVector), glm::normalize(cameraVector)) > minDotValueForBehind)
					{
						newDistance = -newDistance;
					}

					m_distanceField[index] = newDistance;
				}
			}
		}
	}
}

void SignedDistanceField::create3dTexture(int dimension)
{
	std::vector<UINT8> rgbaBuffer(8 * 4);
	std::fill(rgbaBuffer.begin(), rgbaBuffer.end(), 255U);

	// only r
	int i = 0;
	rgbaBuffer[i*4] = 255U;
	rgbaBuffer[i*4+1] = 0;
	rgbaBuffer[i*4+2] = 0;
	rgbaBuffer[i*4+3] = 255U;

	i++;
	rgbaBuffer[i * 4] = 0;
	rgbaBuffer[i * 4 + 1] = 255U;
	rgbaBuffer[i * 4 + 2] = 0;
	rgbaBuffer[i * 4 + 3] = 255U;


	static int textureId = 0;

	glBindTexture(GL_TEXTURE_3D, textureId);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, 2, 2, 2, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, rgbaBuffer.data());
	glBindTexture(GL_TEXTURE_3D, 0);
}
