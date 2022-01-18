#include "pointCloud.h"

PointCloud::PointCloud():
	mesh{},
	generated{false}
{
	size = sizeof(points) / sizeof(points[0]);
}

void PointCloud::fillPointCloud(ofImage& depthImage, float maxDepth)
{
	auto pixels = depthImage.getPixels();
	auto bytes = pixels.getTotalBytes();
	float width = pixels.getWidth();
	float height = pixels.getHeight();
	auto pixelData = pixels.getData();

	int index = 0;
	ofRectangle view;
	view.width = pixels.getWidth();
	view.height = pixels.getHeight();
	
	mesh.setMode(OF_PRIMITIVE_POINTS);
	mesh.clearVertices();

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			auto data = pixelData;
			float linearDepth = (*data) / 255.0F;

			if (linearDepth <= 0.0F || linearDepth >= 1.0F)
			{
				points[index] = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
				index++;
				pixelData += pixels.getBytesPerPixel();
				continue;
			}

			auto transformedDepth = 0.2f + linearDepth * 0.8f;

			const float scale = 10;
			//pointCloud[index] = cameras[0]->cameraToWorld(point, view);
			points[index] = glm::vec4((2.0f * x / width - 1.0f) * transformedDepth * 1.3333F, (2.0f * (height-y) / height - 1.0f) * transformedDepth, -transformedDepth * maxDepth, linearDepth)*glm::vec4(scale, scale, scale, 1);

			index++;
			pixelData += pixels.getBytesPerPixel();
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
