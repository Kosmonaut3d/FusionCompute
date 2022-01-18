#include "sdf.h"

SignedDistanceField::SignedDistanceField(int resolution, glm::vec3 origin, float scale)
{
	this->resolution = resolution;
	resolutionSq = resolution * resolution;
	this->origin = origin;
	this->scale = scale;
	world = ofMatrix4x4::newTranslationMatrix(origin) * ofMatrix4x4::newScaleMatrix(glm::vec3(scale));
	distanceField = std::vector<float>();
	distanceField.resize(resolution * resolution * resolution); 
	std::fill(distanceField.begin(), distanceField.end(), 10000000.0F);
}

void SignedDistanceField::drawOutline()
{
	ofPushStyle();
	ofGetCurrentRenderer()->setFillMode(ofFillFlag::OF_OUTLINE);
	ofSetColor(255, 0, 0);
	ofDrawBox(origin, 1,1,1);

	ofSetColor(200, 100, 200);
	float scalehalf = scale / 2;
	ofDrawBox(origin+glm::vec3(scalehalf, scalehalf, scalehalf), scale, scale, scale);
	ofPopStyle();
}

void SignedDistanceField::drawGrid(float minDistance)
{
	ofPushStyle();
	auto stepsize = scale / resolution;
	auto halfstep = stepsize / 2;

	ofGetCurrentRenderer()->setFillMode(ofFillFlag::OF_OUTLINE);

	ofSetColor(255, 255, 255);

	for (int x = 0; x < resolution; x++)
	{
		for (int y = 0; y < resolution; y++)
		{
			for (int z = 0; z < resolution; z++)
			{
				auto sdfValue = distanceField[getIndexFromXYZ(x, y, z)];

				if (sdfValue > minDistance)
				{
					continue;
				}

				int value = (1.0f - (sdfValue / minDistance))*255;
				ofSetColor(value, value, value);

				auto localMiddleOfBox = origin + glm::vec3(stepsize * x + halfstep, stepsize * y + halfstep, stepsize * z + halfstep);
				ofDrawBox(localMiddleOfBox, stepsize, stepsize, stepsize);
			}
		}
	}
	ofPopStyle();
}

glm::vec3& SignedDistanceField::getXYZFromIndex(int index)
{
	int z = index / resolutionSq;
	index = index % resolutionSq;
	int y = index / resolution;
	index = index % resolution;
	int x = index;
	return glm::vec3(x, y, z);
}

int SignedDistanceField::getIndexFromXYZ(int x, int y, int z)
{
	return z*resolutionSq+y*resolution+x;
}

void SignedDistanceField::insertPoint(glm::vec3& point, glm::vec3& cameraOrigin, float minDotValueForBehind)
{
	if (point == glm::vec3(0, 0, 0))
	{
		return;
	}
	auto stepsize = scale / resolution;
	auto halfstep = stepsize / 2;
	for (int x = 0; x < resolution; x++)
	{
		for (int y = 0; y < resolution; y++)
		{
			for (int z = 0; z < resolution; z++)
			{
				auto index = getIndexFromXYZ(x, y, z);
				auto sdfValue = distanceField[index];
				auto localMiddleOfBox = origin + glm::vec3(stepsize * x + halfstep, stepsize * y + halfstep, stepsize * z + halfstep);

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

					distanceField[index] = newDistance;
				}
			}
		}
	}
}
