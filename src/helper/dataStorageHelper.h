#pragma once
#include "ofMain.h"

/// <summary>
/// Helper functions for storing binary data and/or images
/// </summary>
class DataStorageHelper
{
  public:
	static void storeData(string path, const char* data, int size);
	static void loadData(string path, char* data, int size);

	template <typename T> static void storeData(string path, const T* data, int size);

	template <typename T> static void loadData(string path, T* data, int size);

	static void storeImage(string path, ofImage& image);
	static void loadImage(string path, ofImage& image);
};

template <typename T> inline void DataStorageHelper::storeData(string path, const T* data, int size)
{
	DataStorageHelper::storeData(path, reinterpret_cast<const char*>(data), size * sizeof(T));
}

template <typename T> inline void DataStorageHelper::loadData(string path, T* data, int size)
{
	DataStorageHelper::loadData(path, reinterpret_cast<char*>(data), size * sizeof(T));
}
