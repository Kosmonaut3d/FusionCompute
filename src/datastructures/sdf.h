#pragma once

#include "ofMain.h"

class SignedDistanceField
{
public:
	SignedDistanceField(int resolution, glm::vec3 position, float scale);

	void drawOutline();
	void drawGrid(float minDistance);
    glm::vec3 getXYZFromIndex(int index);
	int getIndexFromXYZ(int x, int y, int z);
    void insertPoint(glm::vec3 point, glm::vec3 cameraOrigin, float dotBackgroundValue);

private:
	int resolution;
	int resolutionSq;
	glm::vec3 origin;
	float scale;
	ofMatrix4x4 world;
	std::vector<float> distanceField;
};

