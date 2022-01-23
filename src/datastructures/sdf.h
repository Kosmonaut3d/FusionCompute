#pragma once

#include "ofMain.h"

class SignedDistanceField
{
public:
	SignedDistanceField(int resolution, glm::vec3 position, float scale, float truncationDistance);

	void drawOutline();
	void drawRaymarch(ofCamera& camera);
	void drawGrid(float minDistance);
	void move(float x, float y, float z);
    glm::vec3 getXYZFromIndex(int index);
	int getIndexFromXYZ(int x, int y, int z);
	void insertPoint(glm::vec3 point, glm::vec3 cameraOrigin, float minDotValueForBehind, float minPointSize);
	void create3dTexture(int dimension, float maxDist);
	void update3dTexture();
	void storeData();

private:
	int m_resolution;
	int m_resolutionSq;
	glm::vec3 m_origin;
	float m_scale;
	float m_truncationDistance;
	float m_recpTruncationDistance;
	float m_distanceBetweenCells;
	ofMatrix4x4 m_world;
	std::vector<float> m_distanceField;
	ofShader m_raymarchShader;

	ofBoxPrimitive m_boxMesh;
};

