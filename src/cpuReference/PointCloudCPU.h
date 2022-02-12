#pragma once
#include <ofMain.h>
#include <ofxKinect.h>

class PointCloudCPU
{
public: 
	PointCloudCPU();

	void fillPointCloud(ofxKinect& kinect, int downsample, bool compNormals, glm::mat4x4 viewToWorld);

	void draw(bool drawNormals, glm::mat4x4 viewToWorld);

	std::vector<glm::vec3>& getPoints();
	std::vector<glm::vec3>& getNormals();
	int getSize();
	ofMesh& getMesh();

private:
	std::vector<glm::vec3> m_points;
	std::vector<glm::vec3> m_normals;
	int m_size;

	ofMesh m_meshPoints;
	ofMesh m_meshNormals;
	bool m_generated;
};

