#include "IterativeClosestPointCPU.h"

//----------------------------------------------------------------------------------------------------------
IterativeClostestPointCPU::IterativeClostestPointCPU()
    : m_epsilonDistance(0.4f)
    , m_epsilonNormal(0.8f)
    , m_failPixel(0)
    , m_failDistance(0)
    , m_failNormal(0)
{
}

void IterativeClostestPointCPU::compute(const glm::vec3* newVertices, const glm::vec3* newNormals,
                                        const glm::vec3* oldVertices, const glm::vec3* oldNormals,
                                        const glm::mat4x4& oldTransform, const glm::mat4x4& oldTransformProjection,
                                        glm::mat4x4& newTransform, const int downscale)
{
	const int WIDTH  = 640 / downscale;
	const int HEIGHT = 480 / downscale;
	const int SIZE   = WIDTH * HEIGHT;

	// Copy
	newTransform = glm::mat4x4(oldTransform);

	/// ..........................
	for (int i = 0; i < SIZE; i++)
	{

		int x, y;
		getXYfromIndex(i, WIDTH, &x, &y);

		glm::vec3 newVertex = newVertices[i];
		glm::vec3 newNormal = newNormals[i];

		solveLinearEquation(newVertex, newNormal, oldVertices, oldNormals, oldTransform, oldTransformProjection,
		                    newTransform, WIDTH, HEIGHT, SIZE);
	}
}

//----------------------------------------------------------------------------------------------------------
bool IterativeClostestPointCPU::solveLinearEquation(const glm::vec3 newVertex,

                                                    const glm::vec3 newNormal,

                                                    const glm::vec3* oldVertices,

                                                    const glm::vec3* oldNormals, const glm::mat4x4& oldTransform,
                                                    const glm::mat4x4& oldTransformProjection,
                                                    glm::mat4x4& newTransform, int w, int h, int size)
{
	if (newVertex.z == 0)
	{
		m_failPixel++;
		return false;
	}

	glm::vec3 ndc = glm::vec4(newVertex, 1) * oldTransform;
	if (ndc.x < -1 || ndc.x > 1 || ndc.y < -1 || ndc.y > 1)
	{
		m_failPixel++;
		return false;
	}

	// OPENGL
	ndc.y = -ndc.y;
	int x = (ndc.x + 1) * w / 2;
	int y = (ndc.y + 1) * h / 2;

	// COORD y = 0-> top
	int index = y * w + x;
	if (index > size)
	{
		return false;
	}

	const glm::vec4 referenceVertex = glm::vec4(oldVertices[index], 1);
	const glm::vec3 referenceNormal = oldNormals[index];

	// Move up
	glm::mat3x3 oldRot =
	    glm::mat3x3(oldTransform[0][0], oldTransform[0][1], oldTransform[0][2], oldTransform[1][0], oldTransform[1][1],
	                oldTransform[1][2], oldTransform[2][0], oldTransform[2][1], oldTransform[2][2]);

	glm::mat3x3 newRot =
	    glm::mat3x3(newTransform[0][0], newTransform[0][1], newTransform[0][2], newTransform[1][0], newTransform[1][1],
	                newTransform[1][2], newTransform[2][0], newTransform[2][1], newTransform[2][2]);

	// World transformation
	glm::vec3 referenceVertexWorld = referenceVertex * oldTransform;
	glm::vec3 referenceNormalWorld = referenceVertex * oldRot;

	glm::vec3 newVertexWorld = glm::vec4(newVertex, 1) * newTransform;
	glm::vec3 newNormalWorld = newNormal * newRot;

	glm::vec3 distance = newVertexWorld - referenceVertexWorld;
	if (glm::length(distance) > m_epsilonDistance)
	{
		m_failDistance++;
		return false;
	}

	if (glm::abs(glm::dot(referenceNormalWorld, newNormalWorld)) > m_epsilonNormal)
	{
		m_failNormal++;
		return false;
	}
	// Point correspondence found! Yay

	// AT _ vec6 transposed
	glm::vec3 at_0 = build_AT(newVertexWorld, referenceNormalWorld);
	glm::vec3 at_1 = referenceNormalWorld;

	float     b      = glm::dot(referenceNormalWorld, (referenceVertexWorld - newVertexWorld));
	glm::vec3 at_b_0 = at_0 * b;
	glm::vec3 at_b_1 = at_1 * b;

	// Matrix ATxA =>
	double row[6] = {at_b_0.x, at_b_0.y, at_b_0.z, at_b_1.x, at_b_1.y, at_b_1.z};
	double A[6][6];
	double L[6][6];

	for (x = 0; x < 6; x++)
	{
		for (y = 0; y < 6; y++)
		{
			A[x][y] = row[x] * row[y];
			L[x][y] = 0;
		}
	}

	// Cholesky Banachiewicz
	for (int i = 0; i < 6; i++)
	{
		for (int j = 0; j <= i; j++)
		{
			double sum = 0.0;
			for (int k = 0; k < j; k++)
				sum += L[i][k] * L[j][k];

			if (i == j)
				L[i][j] = sqrt(A[i][i] - sum);
			else
				L[i][j] = (1.0 / L[j][j] * (A[i][j] - sum));
		}
	}

	// A = L * LT
	// Ax = b
	// L*y = b
	// LT*x = y

	// Cholesky is symmetric!

	return true;
}

//----------------------------------------------------------------------------------------------------------
float IterativeClostestPointCPU::pointPlaneEnergyFunction(glm::vec3 distance, glm::vec3 normal)
{
	// Vector dot (plane) normal = point to plane distance
	const float pointToPlane = dot(distance, normal);

	// L2 norm for single value
	return abs(pointToPlane);
}

//----------------------------------------------------------------------------------------------------------
glm::vec3 IterativeClostestPointCPU::build_AT(glm::vec3 V, glm::vec3 N)
{
	// G =
	// 0  -v3  v2 1 0 0
	// v3  0  -v1 0 1 0
	// -v2 v1   0 0 0 1

	// G_T
	// 0 v3 -v2
	// -v3 0 v1 etc...
	glm::mat3x3 G_T1 = glm::mat3x3(0, -V.z, V.y, V.z, 0, -V.x, -V.y, V.x, 0);

	glm::vec3 out_0 = G_T1 * N;

	// out_1 = N
	return out_0;
}

//----------------------------------------------------------------------------------------------------------
void IterativeClostestPointCPU::getXYfromIndex(int index, int w, int* x, int* y)
{
	*y = index / w;
	*x = index % w;
}