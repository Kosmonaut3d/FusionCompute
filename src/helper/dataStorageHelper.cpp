#include "dataStorageHelper.h"

void DataStorageHelper::storeData(string path, const char* data, int size)
{
	ofstream fout(ofToDataPath(path).c_str(), std::ios::binary);
	fout.write(data, size);
	fout.close();
}

void DataStorageHelper::loadData(string path, char* data, int size)
{
	ifstream fin(ofToDataPath(path).c_str(), ios::binary);
	fin.read(data, size);
	fin.close();
}

void DataStorageHelper::storeImage(string path, ofImage& image)
{
	const auto& pixels = image.getPixels();
	const auto* data = pixels.getData();
	DataStorageHelper::storeData(path, data, pixels.getWidth() * pixels.getHeight());
}

void DataStorageHelper::loadImage(string path, ofImage& image)
{
	auto& pixels = image.getPixels();
	auto* data = pixels.getData();
	DataStorageHelper::loadData(path, data, pixels.getWidth() * pixels.getHeight());
	image.update();
}


