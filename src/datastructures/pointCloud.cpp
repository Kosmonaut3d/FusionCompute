#include "pointCloud.h"

PointCloud::PointCloud():
	mesh{},
	generated{false}
{
	size = sizeof(points) / sizeof(points[0]);
}

void PointCloud::fillPointCloud(ofxKinect& kinect, int downsample)
{
	int w = 640;
	int h = 480;
	mesh.setMode(OF_PRIMITIVE_POINTS);
	mesh.clearVertices();
	mesh.clearColors();
	int step = downsample;
	for (int y = 0; y < h; y += step) {
		for (int x = 0; x < w; x += step) {
			if (kinect.getDistanceAt(x, y) > 0) {
				glm::vec3 pos = kinect.getWorldCoordinateAt(x, y) * 0.01;
				
				int index = y * w + x;
				points[index] = glm::vec4(pos.x, -pos.y, -pos.z, -pos.z);

				mesh.addColor(kinect.getColorAt(x,y));
				mesh.addVertex(points[index]);
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
	int width = pixels.getWidth();
	int height = pixels.getHeight();
	auto pixelData = pixels.getData();
	auto pixelDataSize = pixels.getBytesPerPixel();
	int downsampleSq = (downsample+1);

	int index = 0;
	ofRectangle view;
	view.width = pixels.getWidth();
	view.height = pixels.getHeight();
	
	mesh.setMode(OF_PRIMITIVE_POINTS);
	mesh.clearVertices();
	mesh.clearColors();

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
			points[index] = glm::vec4((2.0f * x / width - 1.0f) * transformedDepth * 1.3333F, (2.0f * (height-y) / height - 1.0f) * transformedDepth, -transformedDepth * maxDepth, linearDepth)*glm::vec4(scale, scale, scale, 1);
		}
	}

	auto size = pixels.getWidth() * pixels.getHeight();

	for (unsigned int i = 0; i < size; i++) {
		mesh.addColor(ofColor::white.getLerped(ofColor::black, 1-points[i].w));
		mesh.addVertex(points[i]);
	}
	generated = true;
}

/// <summary>
/// 
/// </summary>
void PointCloud::draw()
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
	mesh.drawVertices();
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
	return mesh;
}
