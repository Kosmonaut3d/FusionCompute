#include "pointCloud.h"

PointCloud::PointCloud() :
	meshPoints{},
	generated{ false }
{
	size = sizeof(points) / sizeof(points[0]);
	meshPoints.setMode(OF_PRIMITIVE_POINTS);
	meshNormals.setMode(OF_PRIMITIVE_LINES);
}

void PointCloud::fillPointCloud(ofxKinect& kinect, int downsample, bool compNormals)
{
	int w = 640;
	int h = 480;
	meshPoints.clearVertices();
	meshPoints.clearColors();
	int step = downsample;


	if (!compNormals)
	{
		for (int y = 0; y < h; y += step) {
			for (int x = 0; x < w; x += step) {
				if (kinect.getDistanceAt(x, y) > 0) {
					glm::vec3 pos = kinect.getWorldCoordinateAt(x, y) * 0.01;

					int index = y * w + x;
					points[index] = glm::vec4(pos.x, -pos.y, -pos.z, -pos.z);

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
					glm::vec3 pos = kinect.getWorldCoordinateAt(x, y) * glm::vec3(1, -1, -1);

					glm::vec3 pos_x = kinect.getWorldCoordinateAt(x+1, y) * glm::vec3(1, -1, -1);
					glm::vec3 pos_y = kinect.getWorldCoordinateAt(x, y+1) * glm::vec3(1, -1, -1);

					int index = y * w + x;
					points[index] = glm::vec4(pos * 0.01, 1);

					meshPoints.addColor(kinect.getColorAt(x, y));
					meshPoints.addVertex(points[index]);

					const auto v1 = glm::normalize(pos_x - pos);
					const auto v2 = glm::normalize(pos_y - pos);
					glm::vec3 nor = glm::cross(v2, v1);

					glm::vec3 color = (nor + glm::vec3(1, 1, 1)) * 0.5 * 255;

					meshNormals.addColor(ofColor(color.x, color.y, color.z));
					meshNormals.addVertex(pos * 0.01);
					meshNormals.addColor(ofColor(color.x, color.y, color.z));
					meshNormals.addVertex((pos * 0.01 + nor));
				}
			}
		}
	}

	//auto size = w*h;

	//for (unsigned int i = 0; i < size; i++) {
	//	mesh.addColor(ofColor::white.getLerped(ofColor::black, 1 - points[i].w));
	//	mesh.addVertex(points[i]);
	//}
	generated = true;
}

void PointCloud::fillPointCloud(ofImage& depthImage, float maxDepth, int downsample)
{
	auto pixels = depthImage.getPixels();
	auto bytes = pixels.getTotalBytes();
	auto width = pixels.getWidth();
	auto height = pixels.getHeight();
	auto pixelData = pixels.getData();
	auto pixelDataSize = pixels.getBytesPerPixel();
	int downsampleSq = (downsample + 1);

	int index = 0;
	ofRectangle view;
	view.width = pixels.getWidth();
	view.height = pixels.getHeight();

	meshPoints.setMode(OF_PRIMITIVE_POINTS);
	meshPoints.clearVertices();
	meshPoints.clearColors();

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			int index = y * width + x;
			auto data = pixelData[index];
			float linearDepth = static_cast<char>(data) / 255.0F;

			// downsample
			bool skip = !(!downsample || x % downsampleSq == 0 && y % downsampleSq == 0);

			if (linearDepth <= 0.0F || linearDepth >= 1.0F || skip)
			{
				points[index] = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
				continue;
			}

			auto transformedDepth = 0.2f + linearDepth * 0.8f;

			const float scale = 10;
			//pointCloud[index] = cameras[0]->cameraToWorld(point, view);
			points[index] = glm::vec4((2.0f * x / width - 1.0f) * transformedDepth * 1.3333F, (2.0f * (height - y) / height - 1.0f) * transformedDepth, -transformedDepth * maxDepth, linearDepth) * glm::vec4(scale, scale, scale, 1);
		}
	}

	auto size = pixels.getWidth() * pixels.getHeight();

	for (unsigned int i = 0; i < size; i++) {
		meshPoints.addColor(ofColor::white.getLerped(ofColor::black, 1 - points[i].w));
		meshPoints.addVertex(points[i]);
	}
	generated = true;
}

/// <summary>
/// 
/// </summary>
void PointCloud::draw(bool drawNormals)
{
	if (!generated)
	{
		return;
	}
	glPointSize(3);
	ofPushMatrix();
	float factor = 1;
	ofScale(factor, factor, factor);
	// the projected points are 'upside down' and 'backwards' 
	ofEnableDepthTest();

	if (drawNormals)
	{
		meshNormals.drawVertices();
	}
	else
	{
		meshPoints.drawVertices();
	}

	ofDisableDepthTest();
	ofPopMatrix();
}

/// <summary>
/// 
/// </summary>
/// <returns></returns>
glm::vec4* PointCloud::getPoints()
{
	return points;
}

/// <summary>
/// 
/// </summary>
/// <returns></returns>
int PointCloud::getSize()
{
	return size;
}

ofMesh& PointCloud::getMesh()
{
	return meshPoints;
}
