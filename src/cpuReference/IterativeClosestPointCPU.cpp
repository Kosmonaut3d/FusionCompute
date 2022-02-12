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

void IterativeClostestPointCPU::compute(const std::vector<glm::vec3>& newVertices,

                                        const std::vector<glm::vec3>& newNormals,

                                        const std::vector<glm::vec3>& oldVertices,
                                        const std::vector<glm::vec3>& oldNormals, const glm::mat4x4 & worldToViewOld,
                                        const glm::mat4x4& projection, glm::mat4x4 & worldToViewNew, const int downscale)
{
	m_failPixel    = 0;
	m_failDistance = 0;
	m_failNormal   = 0;

	const int WIDTH  = 640 / downscale;
	const int HEIGHT = 480 / downscale;
	const int W2     = WIDTH / 2;
	const int H2     = HEIGHT / 2;
	const int SIZE   = WIDTH * HEIGHT;

	// Copy

	// https://gist.github.com/podgorskiy/04a3cb36a27159e296599183215a71b0

	glm::mat4x4 viewToWorld_prev    = glm::inverse(worldToViewOld);
	glm::mat4x4 viewProjection_prev = projection * worldToViewOld;
	glm::mat3x3 viewToWorldRot_prev = glm::mat3x3(viewToWorld_prev);

	glm::mat4x4 viewToWorld_iter = glm::inverse(worldToViewOld);
	worldToViewNew               = worldToViewOld;
	static int  computationCount = 0;
	computationCount++;

	int MAX_IT = 10;
	for (int it = 0; it < MAX_IT; it++)
	{

		glm::mat3x3 viewToWorldRot_iter = glm::mat3x3(viewToWorld_iter);

		/* Eigen::Matrix4Xf eiProjection   = GLM2E<float, 4, 4>(projection);
		glm::mat4x4      glmProjections       = E2GLM<float, 4, 4>(eiProjection);

		glm::mat4x4 test = glm::inverse(glmProjections)  * projection;
		*/

		Eigen::Matrix<double, 6, 1> A_T_sum;
		A_T_sum.setZero();
		double b_sum = 0;

		double E_sum = 0.0;

		int found = 0;
		/// ..........................
		for (int i = 0; i < SIZE; i++)
		{
			// TEST MANIPULATION
			glm::vec3 newVertexWorld = viewToWorld_iter * glm::vec4(newVertices[i], 1);

			// TODO FIX
			// Check if there was a valid depth at that vertex
			if (newVertexWorld.z == 0)
			{
				m_failPixel++;
				continue;
			}

			// NOTE: transform from world to camera -> project
			glm::vec4 clipSpacePos = viewProjection_prev * glm::vec4(newVertexWorld, 1);

			/* if (clipSpacePos.w <= 0)
			{
				m_failPixel++;
				continue;
			}*/

			glm::vec3 ndc = glm::vec3(clipSpacePos) / clipSpacePos.w;
			if (ndc.x < -1 || ndc.x > 1 || ndc.y < -1 || ndc.y > 1)
			{
				m_failPixel++;
				continue;
			}

			// OPENGL
			ndc.y      = -ndc.y;
			int x_proj = round((ndc.x + 1) * W2);
			int y_proj = round((ndc.y + 1) * H2);

			int index = y_proj * WIDTH + x_proj;
			if (index > SIZE)
			{
				continue;
			}

			// TODO Delete
			int x_ori, y_ori;
			getXYfromIndex(i, WIDTH, &x_ori, &y_ori);

			// transform to camera rotation
			glm::vec3 newNormalWorld = viewToWorldRot_iter * newNormals[i];

			// Reference Vertices in world space
			const glm::vec3 oldVertex = oldVertices[index];

			// TODO: FIX should be before world transform
			if (oldVertex.z == 0)
			{
				m_failPixel++;
				continue;
			}

			if (abs(x_ori - x_proj) > 2 || abs(y_ori - y_proj) > 2)
			{
				int test = 1;
			}

			const glm::vec3 referenceVertexWorld = viewToWorld_prev * glm::vec4(oldVertex, 1);
			const glm::vec3 referenceNormalWorld = viewToWorldRot_prev * oldNormals[index];

			glm::vec3 distance = newVertexWorld - referenceVertexWorld;
			if (glm::length(distance) > m_epsilonDistance)
			{
				m_failDistance++;
				continue;
			}

			if (glm::dot(referenceNormalWorld, newNormalWorld) < m_epsilonNormal)
			{
				m_failNormal++;
				continue;
			}

			float E = abs(dot(distance, referenceNormalWorld));
			E_sum += E*E;
			found++;

			float v1 = newVertexWorld.x;
			float v2 = newVertexWorld.y;
			float v3 = newVertexWorld.z;

			// Build G(u)
			Eigen::Matrix<float, 3, 6> G_U{
			    {0.f, -v3, v2, 1.f, 0.f, 0.f}, {v3, 0.f, -v1, 0.f, 1.f, 0.f}, {-v2, v1, 0.f, 0.f, 0.f, 1.f}}; // OK

			Eigen::Matrix<float, 6, 3> G_U_T = G_U.transpose(); // OK

			Eigen::Vector3f Ng     = GLM2E<float, 3>(referenceNormalWorld);
			Eigen::Vector3f Vg_old = GLM2E<float, 3>(referenceVertexWorld);
			Eigen::Vector3f Vg_it  = GLM2E<float, 3>(newVertexWorld);

			Eigen::Matrix<float, 6, 1> A_T = G_U_T * Ng;

			float b = Ng.transpose() * (Vg_old - Vg_it);

			A_T_sum += A_T.cast<double>();
			b_sum += b;
		}

		printf("Iteration: %d E_sum %f, matches %d\n", it, E_sum, found);

		/* if (E_sum < 1)
		{
			return;
		}*/

		Eigen::Matrix<double, 1, 6> A_sum = A_T_sum.transpose();

		Eigen::Matrix<double, 6, 6> A_T_A = A_T_sum * A_sum;

		Eigen::Matrix<double, 6, 1> A_b = A_T_sum * b_sum;

		//Eigen::JacobiSVD<Eigen::Matrix<double, 6, 6>> svd(A_T_A, Eigen::ComputeFullU | Eigen::ComputeFullV);
		//Eigen::Matrix<double, 6, 1> result = svd.solve(A_b);
		Eigen::Matrix<double, 6, 1> result = A_T_A.bdcSvd(Eigen::ComputeFullU | Eigen::ComputeFullV).solve(A_b);
		//Eigen::Matrix<double, 6, 1> result = A_T_A.llt().solve(A_b);

		if (!isnan(result[0]))
		{
			int test = 1;

			float a = result[0];
			float b = result[1];
			float g = result[2];
			float tx = result[3];
			float ty = result[4];
			float tz = result[5];

			// Update the transformation matrix
			Eigen::Matrix4f T_inc{{1, a, -g, tx}, {-a, 1, b, ty}, {g, -b, 1, tz}, {0, 0, 0, 1}};
			viewToWorld_iter = E2GLM(T_inc) * viewToWorld_iter;
			worldToViewNew   = glm::inverse(viewToWorld_iter);

			// Calculate E function:

		}
	}

	return;
}

//----------------------------------------------------------------------------------------------------------
bool IterativeClostestPointCPU::solveLinearEquation(const glm::vec3 newVertex,

                                                    const glm::vec3 newNormal,

                                                    const glm::vec3* oldVertices,

                                                    const glm::vec3* oldNormals, const glm::mat4x4& oldTransform,
                                                    const glm::mat4x4& oldTransformProjection,
                                                    glm::mat4x4& newTransform, int w, int h, int size)
{

	// Check if the vertex, projected onto our screen by our old MVP is visible on screen even
	/*

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

	*/
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
