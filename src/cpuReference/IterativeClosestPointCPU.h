#pragma once
#include <ofMain.h>

class IterativeClostestPointCPU
{
  public:
	IterativeClostestPointCPU();

	void compute(const glm::vec3* newVertices, const glm::vec3* newNormals, const glm::vec3* oldVertices,
	             const glm::vec3* oldNormals, const glm::mat4x4& oldTransform,
	             const glm::mat4x4& oldTransformProjection, glm::mat4x4& newTransform, const int downscale);

  private:
	bool  solveLinearEquation(const glm::vec3 newVertex, const glm::vec3 newNormal,
 const glm::vec3* oldVertices,
	                          const glm::vec3* oldNormals,
 const glm::mat4x4& oldTransform,
	                          const glm::mat4x4& oldTransformProjection, glm::mat4x4& newTransform, int w, int h, int size);
	float pointPlaneEnergyFunction(glm::vec3 distance, glm::vec3 normal);

	glm::vec3 build_AT(glm::vec3 V, glm::vec3 N);

	void getXYfromIndex(int index, int w, int* x, int* y);

  private:
	float m_epsilonDistance;
	float m_epsilonNormal;
	int   m_failPixel;
	int   m_failDistance;
	int   m_failNormal;
};
