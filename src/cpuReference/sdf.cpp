#include "sdf.h"
#include <algorithm>

SignedDistanceField::SignedDistanceField(int resolution, glm::vec3 origin, float scale, float truncationDistance)
    : m_boxMesh(scale, scale, scale)
{
	setResolution(resolution);
	m_origin                 = origin;
	m_scale                  = scale;
	m_truncationDistance     = truncationDistance;
	m_recpTruncationDistance = 1 / m_truncationDistance;

	m_world    = ofMatrix4x4::newScaleMatrix(scale, scale, scale) * ofMatrix4x4::newTranslationMatrix(origin);
	m_worldInv = m_world.getInverse();

	if (!m_raymarchShader.load("shaders/vertShader.vert", "shaders/rayMarchSDF.frag"))
	{
		throw std::exception(); //"could not load shaders");
	}

	const bool load = false;
	if (load)
	{
		ifstream fin(ofToDataPath("test.bin").c_str(), ios::binary);
		fin.read(reinterpret_cast<char*>(m_distanceField.data()), m_distanceField.size() * sizeof(float));
		fin.close();
		create3dTexture(m_resolution, 10.0f);
	}
}

void SignedDistanceField::drawOutline()
{
	ofPushStyle();
	ofGetCurrentRenderer()->setFillMode(ofFillFlag::OF_OUTLINE);
	ofSetColor(255, 0, 0);
	// ofDrawBox(m_origin, 1,1,1);
	ofVec3f vec = ofVec3f(0, 0, 0);
	vec         = ofVec3f(m_origin) * m_worldInv;
	ofDrawBox(vec, 1, 1, 1);

	ofSetColor(200, 100, 200);
	float scalehalf = m_scale / 2;
	ofDrawBox(m_origin + glm::vec3(scalehalf, scalehalf, scalehalf), m_scale, m_scale, m_scale);
	ofPopStyle();
}

void SignedDistanceField::drawRaymarch(ofCamera& camera, unsigned int texID)
{
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	ofPushStyle();
	m_raymarchShader.begin();
	// glPolygonMode(GL_BACK, GL_LINE);
	float scalehalf = m_scale / 2;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, texID);

	m_raymarchShader.setUniform3f("cameraWorld", camera.getPosition());
	m_raymarchShader.setUniformMatrix4f("sdfBaseTransform", m_worldInv);
	m_raymarchShader.setUniform1f("sdfResolution", m_resolution);
	auto    vp  = ofMatrix4x4(camera.getProjectionMatrix() * camera.getModelViewMatrix());
	ofVec4f ori = ofVec4f(0, 0, 0, 1.0);
	ori         = vp * ori;
	auto res    = camera.worldToCamera(ofVec3f(0, 0, 0));
	m_raymarchShader.setUniformMatrix4f("viewprojection", camera.getModelViewProjectionMatrix());
	m_raymarchShader.setUniform1f("near", camera.getNearClip());
	m_raymarchShader.setUniform1f("far", camera.getFarClip());

	ofDrawBox(m_origin + glm::vec3(scalehalf, scalehalf, scalehalf), m_scale, m_scale, m_scale);
	m_raymarchShader.end();
	ofPopStyle();

	glDisable(GL_CULL_FACE);
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

				int value = (1.0f - (sdfValue / minDistance)) * 255;
				ofSetColor(value, value, value);

				auto localMiddleOfBox =
				    m_origin + glm::vec3(stepsize * x + halfstep, stepsize * y + halfstep, stepsize * z + halfstep);
				ofDrawBox(localMiddleOfBox, stepsize, stepsize, stepsize);
			}
		}
	}
	ofPopStyle();
}

void SignedDistanceField::move(float x, float y, float z)
{
	// auto translation = ofMatrix4x4::newTranslationMatrix(glm::vec4(x, y, z, 0));
	// m_origin.mul;
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
	return z * m_resolutionSq + y * m_resolution + x;
}

void SignedDistanceField::insertPoint(glm::vec3 point, glm::vec3 cameraOrigin, float minDotValueForBehind,
                                      float minPointSize)
{
	if (point == glm::vec3(0, 0, 0))
	{
		return;
	}

	glm::vec3 pointTransformed   = point - m_origin;
	glm::vec3 cameraVector       = point - cameraOrigin;
	float     truncationDistance = m_truncationDistance - minPointSize;

	// better to transform with
	// vec = ofVec3f(m_origin) * m_worldInv;

	if (pointTransformed.x < 0 || pointTransformed.y < 0 || pointTransformed.z < 0 || pointTransformed.x > m_scale ||
	    pointTransformed.y > m_scale || pointTransformed.z > m_scale)
	{
		return;
	}

	auto stepsize = m_scale / m_resolution;
	auto halfstep = stepsize / 2;

	int truncatedSteps = static_cast<int>((truncationDistance * 2.0) / stepsize);

	int minX = max(static_cast<int>((pointTransformed.x - halfstep - truncationDistance) / stepsize), 0);
	int maxX = min(minX + truncatedSteps, m_resolution);

	int minY = max(static_cast<int>((pointTransformed.y - halfstep - truncationDistance) / stepsize), 0);
	int maxY = min(minY + truncatedSteps, m_resolution);
	// TODO: One vector subtraction is faster maybe
	int minZ = max(static_cast<int>((pointTransformed.z - halfstep - truncationDistance) / stepsize), 0);
	int maxZ = min(minX + truncatedSteps, m_resolution);

	for (int x = minX; x < maxX; x++)
	{
		float xPos = stepsize * x + halfstep;

		for (int y = minY; y < maxY; y++)
		{
			float yPos = stepsize * y + halfstep;

			for (int z = minZ; z < maxZ; z++)
			{

				float zPos = stepsize * z + halfstep;

				if (abs(zPos - pointTransformed.z) > truncationDistance)
				{
					continue;
				}

				auto index            = getIndexFromXYZ(x, y, z);
				auto sdfValue         = m_distanceField[index];
				auto localMiddleOfBox = glm::vec3(xPos, yPos, zPos);

				auto gridVector = localMiddleOfBox - pointTransformed;

				auto newDistance = min(glm::length(gridVector) - minPointSize, m_truncationDistance);

				if (newDistance < abs(sdfValue))
				{
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

void SignedDistanceField::create3dTexture(int dimension, float maxDist)
{
	int size = dimension * dimension * dimension;

	glGenTextures(1, &m_textureID);
	glBindTexture(GL_TEXTURE_3D, m_textureID);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, dimension, dimension, dimension, 0, GL_RED, GL_FLOAT,
	             m_distanceField.data());
}

void SignedDistanceField::update3dTexture()
{
	create3dTexture(m_resolution, 10.0f);
}

void SignedDistanceField::storeData()
{
	return;
	ofstream fout(ofToDataPath("test.bin").c_str(), std::ios::binary);
	fout.write(reinterpret_cast<char*>(m_distanceField.data()), m_distanceField.size() * sizeof(float));
	fout.close();
}

void SignedDistanceField::resetData()
{
	std::fill(m_distanceField.begin(), m_distanceField.end(), m_truncationDistance);
}

void SignedDistanceField::setResolution(int resolution)
{
	if (resolution == m_resolution)
	{
		return;
	}
	m_resolution           = resolution;
	m_resolutionSq         = resolution * resolution;
	m_distanceBetweenCells = m_scale / resolution;
	m_distanceField        = std::vector<float>(resolution * resolution * resolution);
	resetData();
}

// Returns inverted ModelWorldProjection
ofMatrix4x4& SignedDistanceField::getInvWorld()
{
	return m_worldInv;
}

unsigned int SignedDistanceField::getTextureID()
{
	return m_textureID;
}
