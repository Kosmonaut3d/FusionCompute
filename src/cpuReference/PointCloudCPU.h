#pragma once
#include <ofMain.h>
#include <ofxKinect.h>

class PointCloudCPU
{
public: 
	PointCloudCPU();

	void fillPointCloud(ofxKinect& kinect, int downsample, bool compNormals);

	void draw(bool drawNormals);

	glm::vec3* getPoints();
	glm::vec3* getNormals();
	int getSize();
	ofMesh& getMesh();

private:
	glm::vec3 m_points[640 * 480];
	glm::vec3 m_normals[640 * 480 * 2];
	int m_size;

	ofMesh m_meshPoints;
	ofMesh m_meshNormals;
	bool m_generated;
};

