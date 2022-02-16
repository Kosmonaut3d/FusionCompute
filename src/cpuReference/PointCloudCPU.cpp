#include "PointCloudCPU.h"
#include <scenes/GUIScene.h>

//----------------------------------------------------------------------------------------------------------
PointCloudCPU::PointCloudCPU()
    : m_points(640 * 480)
    , m_normals(640 * 480 * 2)
    , m_meshPoints{}
    , m_generated{false}
{
	m_size = 640 * 480;
	m_meshPoints.setMode(OF_PRIMITIVE_POINTS);
	m_meshNormals.setMode(OF_PRIMITIVE_LINES);
}

//----------------------------------------------------------------------------------------------------------
void PointCloudCPU::fillPointCloud(ofxKinect& kinect, int downsample, bool compNormals, glm::mat4x4 viewToWorld,
                                   bool worldSpace)
{
	const int w = 640 / downsample;
	const int h = 480 / downsample;

	int step = downsample;

	float scaleToMeters = .001;
	// 0.001;
	glm::vec3 trafo = glm::vec3(1, -1, -1) * scaleToMeters;

	glm::vec3 zero = glm::vec3(0, 0, 0);
	std::fill(m_points.begin(), m_points.end(), zero);
	m_meshPoints.clearVertices();
	m_meshPoints.clearColors();

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

					glm::vec3 pos = kinect.getWorldCoordinateAt(x_, y_) * trafo;

					if (worldSpace)
					{
						pos = viewToWorld * glm::vec4(pos, 1);
					}

					m_points[index] = pos;

					m_meshPoints.addColor(kinect.getColorAt(x_, y_));
					m_meshPoints.addVertex(pos);
				}
			}
		}
	}
	else
	{
		std::fill(m_normals.begin(), m_normals.end(), zero);
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

					if (worldSpace)
					{
						pos   = viewToWorld * glm::vec4(pos, 1);
						pos_x = viewToWorld * glm::vec4(pos_x, 1);
						pos_y = viewToWorld * glm::vec4(pos_y, 1);
					}

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
void PointCloudCPU::draw(bool drawNormals, glm::mat4x4 viewToWorld)
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
std::vector<glm::vec3>& PointCloudCPU::getPoints()
{
	return m_points;
}

//----------------------------------------------------------------------------------------------------------
std::vector<glm::vec3>& PointCloudCPU::getNormals()
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
