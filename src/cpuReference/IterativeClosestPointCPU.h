#pragma once
#include <ofMain.h>
#include "helper/eigen2glm.h"

class IterativeClostestPointCPU
{
  public:
	IterativeClostestPointCPU();

	void compute(const std::vector<glm::vec3>&,
 const std::vector<glm::vec3>& newNormals,
	             const std::vector<glm::vec3>& oldVertices,
 const std::vector<glm::vec3>& oldNormals, const glm::mat4x4 & worldToViewOld, const glm::mat4x4& Projection, glm::mat4x4& worldToViewNew,
	             const int downscale);

	void getCorrespondences(const std::vector<glm::vec3>& oldVertices,
 const std::vector<glm::vec3>& oldNormals,
	                        const std::vector<glm::vec3>& newVertices, const std::vector<glm::vec3>& newNormals,
 glm::mat4x4& viewToWorld_prev,
	                        glm::mat3x3& viewToWorldRot_prev, glm::highp_dmat4& viewToWorld_iter, glm::mat3x3& viewToWorldRot_iter,
 glm::highp_dmat4& worldToView_iter,
	                        const glm::mat4x4& projection,
 const int& WIDTH,
	                        const int& W2,
 const int& H2,
	                        const int& SIZE,
 int& fail_z, double& E_sum,
	                        int& found, std::vector<std::tuple<glm::vec3, glm::vec3, glm::vec3>>& correspondences);

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
