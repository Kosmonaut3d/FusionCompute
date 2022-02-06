#include "PointCloudCPU.h"
#include <scenes/GUIScene.h>

//----------------------------------------------------------------------------------------------------------
PointCloudCPU::PointCloudCPU() :
	meshPoints{},
	generated{ false }
{
	size = sizeof(points) / sizeof(points[0]);
	meshPoints.setMode(OF_PRIMITIVE_POINTS);
	meshNormals.setMode(OF_PRIMITIVE_LINES);
}

//----------------------------------------------------------------------------------------------------------
void PointCloudCPU::fillPointCloud(ofxKinect& kinect, int downsample, bool compNormals)
{
	const int w = 640;
	const int h = 480;

	meshPoints.clearVertices();
	meshPoints.clearColors();
	int step = downsample;

	float     scaleToMeters = 0.001;
	glm::vec3 trafo         = glm::vec3(1, -1, -1) * scaleToMeters;

	if (!compNormals)
	{
		for (int y = 0; y < h; y += step) {
			for (int x = 0; x < w; x += step) {
				if (kinect.getDistanceAt(x, y) > 0) {
					glm::vec3 pos = kinect.getWorldCoordinateAt(x, y) * trafo;

					int index = y * w + x;
					points[index] = glm::vec4(pos.x, pos.y, pos.z, 1);

					meshPoints.addColor(kinect.getColorAt(x, y));
					meshPoints.addVertex(points[index]);
				}
			}
		}
	}
	else
	{
		meshNormals.clearVertices();
		meshNormals.clearColors();
		for (int y = 0; y < h; y += step) {
			for (int x = 0; x < w; x += step) {
				if (kinect.getDistanceAt(x, y) > 0 && kinect.getDistanceAt(x+1, y) > 0 && kinect.getDistanceAt(x, y+1) > 0){
					glm::vec3 pos = kinect.getWorldCoordinateAt(x, y) * trafo;

					glm::vec3 pos_x = kinect.getWorldCoordinateAt(x + 1, y) * trafo;
					glm::vec3 pos_y = kinect.getWorldCoordinateAt(x, y + 1) * trafo;

					int index = y * w + x;
					points[index] = glm::vec4(pos, 1);

					meshPoints.addColor(kinect.getColorAt(x, y));
					meshPoints.addVertex(points[index]);

					const auto v1 = glm::normalize(pos_x - pos);
					const auto v2 = glm::normalize(pos_y - pos);
					glm::vec3 nor = glm::cross(v2, v1);

					glm::vec3 color = (nor + glm::vec3(1, 1, 1)) * 0.5 * 255;

					meshNormals.addColor(ofColor(color.x, color.y, color.z));
					meshNormals.addVertex(pos);
					meshNormals.addColor(ofColor(color.x, color.y, color.z));
					meshNormals.addVertex((pos + nor));
				}
			}
		}
	}
	generated = true;
}

//----------------------------------------------------------------------------------------------------------
void PointCloudCPU::draw(bool drawNormals)
{
	if (!generated)
	{
		return;
	}
	glPointSize(GUIScene::s_pointCloudDownscale);

	if (drawNormals)
	{
		meshNormals.drawVertices();
	}
	else
	{
		meshPoints.drawVertices();
	}

}

//----------------------------------------------------------------------------------------------------------
glm::vec4* PointCloudCPU::getPoints()
{
	return points;
}

//----------------------------------------------------------------------------------------------------------
int PointCloudCPU::getSize()
{
	return size;
}

//----------------------------------------------------------------------------------------------------------
ofMesh& PointCloudCPU::getMesh()
{
	return meshPoints;
}
