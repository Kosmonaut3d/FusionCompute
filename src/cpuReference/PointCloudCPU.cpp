#include "PointCloudCPU.h"
#include <scenes/GUIScene.h>

//----------------------------------------------------------------------------------------------------------
PointCloudCPU::PointCloudCPU()
    : m_meshPoints{}
    , m_generated{false}
{
	m_size = sizeof(m_points) / sizeof(m_points[0]);
	m_meshPoints.setMode(OF_PRIMITIVE_POINTS);
	m_meshNormals.setMode(OF_PRIMITIVE_LINES);
}

//----------------------------------------------------------------------------------------------------------
void PointCloudCPU::fillPointCloud(ofxKinect& kinect, int downsample, bool compNormals)
{
	const int w = 640 / downsample;
	const int h = 480 / downsample;

	m_meshPoints.clearVertices();
	m_meshPoints.clearColors();
	int step = downsample;

	float     scaleToMeters = 0.001;
	glm::vec3 trafo         = glm::vec3(1, -1, -1) * scaleToMeters;

	if (!compNormals)
	{
		for (int y = 0; y < h; y++)
		{
			for (int x = 0; x < w; x++)
			{
				int x_ = x * step;
				int y_ = y * step;
				if (kinect.getDistanceAt(x_, y_) > 0)
				{
					int index = y * w + x;

					glm::vec3 pos   = kinect.getWorldCoordinateAt(x_, y_) * trafo;
					m_points[index] = pos;

					m_meshPoints.addColor(kinect.getColorAt(x_, y_));
					m_meshPoints.addVertex(pos);
				}
			}
		}
	}
	else
	{
		m_meshNormals.clearVertices();
		m_meshNormals.clearColors();

		for (int y = 0; y < h; y++)
		{
			for (int x = 0; x < w; x++)
			{
				int x_ = x * step;
				int y_ = y * step;

				if (kinect.getDistanceAt(x_, y_) > 0 && kinect.getDistanceAt(x_ + 1, y_) > 0 &&
				    kinect.getDistanceAt(x_, y_ + 1) > 0)
				{
					int index = y * w + x;

					glm::vec3 pos = kinect.getWorldCoordinateAt(x_, y_) * trafo;

					glm::vec3 pos_x = kinect.getWorldCoordinateAt(x_ + 1, y_) * trafo;
					glm::vec3 pos_y = kinect.getWorldCoordinateAt(x_, y_ + 1) * trafo;

					m_points[index] = pos;

					m_meshPoints.addColor(kinect.getColorAt(x_, y_));
					m_meshPoints.addVertex(m_points[index]);

					const auto v1    = (pos_x - pos);
					const auto v2    = (pos_y - pos);
					glm::vec3  nor   = glm::normalize(glm::cross(v2, v1));
					m_normals[index] = nor;

					glm::vec3 color = (nor + glm::vec3(1, 1, 1)) * 0.5 * 255;

					m_meshNormals.addColor(ofColor(color.x, color.y, color.z));
					m_meshNormals.addVertex(pos);
					m_meshNormals.addColor(ofColor(color.x, color.y, color.z));
					m_meshNormals.addVertex((pos + nor * 0.01)); // Length = 0.01m/1cm
				}
			}
		}
	}
	m_generated = true;
}

//----------------------------------------------------------------------------------------------------------
void PointCloudCPU::draw(bool drawNormals)
{
	if (!m_generated)
	{
		return;
	}
	glPointSize(GUIScene::s_pointCloudDownscale);

	if (drawNormals)
	{
		m_meshNormals.drawVertices();
	}
	else
	{
		m_meshPoints.drawVertices();
	}
}

//----------------------------------------------------------------------------------------------------------
glm::vec3* PointCloudCPU::getPoints()
{
	return m_points;
}

//----------------------------------------------------------------------------------------------------------
glm::vec3* PointCloudCPU::getNormals()
{
	return m_normals;
}

//----------------------------------------------------------------------------------------------------------
int PointCloudCPU::getSize()
{
	return m_size;
}

//----------------------------------------------------------------------------------------------------------
ofMesh& PointCloudCPU::getMesh()
{
	return m_meshPoints;
}
