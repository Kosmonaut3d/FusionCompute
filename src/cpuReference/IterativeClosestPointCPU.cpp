#include "IterativeClosestPointCPU.h"
#include "scenes/GUIScene.h"

//----------------------------------------------------------------------------------------------------------
IterativeClostestPointCPU::IterativeClostestPointCPU()
    : m_epsilonDistance(0.1f) // TJODDDDDDDDDDDDDDDDD
    , m_epsilonNormal(0.8f)
    , m_failPixel(0)
    , m_failDistance(0)
    , m_failNormal(0)
{
}

// https://eigen.tuxfamily.org/dox/TopicMultiThreading.html
void IterativeClostestPointCPU::compute(const std::vector<glm::vec3>& newVertices,

                                        const std::vector<glm::vec3>& newNormals,

                                        const std::vector<glm::vec3>& oldVertices,
                                        const std::vector<glm::vec3>& oldNormals, const glm::mat4x4& worldToViewOld,
                                        const glm::mat4x4& projection, glm::mat4x4& worldToView_out,
                                        const int downscale)
{
	const int WIDTH  = 640 / downscale;
	const int HEIGHT = 480 / downscale;
	const int W2     = WIDTH / 2;
	const int H2     = HEIGHT / 2;
	const int SIZE   = WIDTH * HEIGHT;

	m_epsilonDistance = GUIScene::s_ICP_epsilonDist;
	m_epsilonNormal   = GUIScene::s_ICP_epsilonNor;

	// Copy

	// https://gist.github.com/podgorskiy/04a3cb36a27159e296599183215a71b0

	///\brief T_g_k-1
	glm::mat4x4 viewToWorld_prev = glm::inverse(worldToViewOld);

	///\brief pi * T_g_k
	// glm::mat4x4 viewProjection_prev = worldToViewOld * projection;

	glm::mat3x3 viewToWorldRot_prev = glm::mat3x3(viewToWorld_prev);

	glm::mat<4, 4, double, glm::precision::highp> viewToWorld_iter = glm::inverse(worldToViewOld);
	glm::mat<4, 4, double, glm::precision::highp> worldToView_iter = worldToViewOld;
	static int                                    computationCount = 0;
	computationCount++;

	double E_sum_first = 0.0;

	int MAX_IT = 10;
	for (int it = 0; it < MAX_IT; it++)
	{
		m_failPixel    = 0;
		m_failDistance = 0;
		m_failNormal   = 0;
		int fail_z     = 0;

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

		// TODO Preallocate maybe
		std::vector<std::tuple<glm::vec3, glm::vec3, glm::vec3>> correspondences; // curr, prev, prev_nor, all world
		getCorrespondences(oldVertices, oldNormals, newVertices, newNormals, viewToWorld_prev, viewToWorldRot_prev,
		                   viewToWorld_iter, viewToWorldRot_iter, worldToView_iter, projection, WIDTH, W2, H2, SIZE,
		                   fail_z, E_sum, found, correspondences);

		printf("Iteration: %d E_sum %f, matches %d, fail %d, fail dist %d, fail nor %d\n", it, E_sum, found,
		       m_failPixel, m_failDistance, m_failNormal);

		if (it == 0)
		{
			E_sum_first = E_sum;
		}
		else
		{
			if (E_sum >= E_sum_first)
			{
				// abort!
				// printf("Iteration: went the wrong way!\n", it, E_sum, found, m_failPixel, m_failDistance,
				// m_failNormal);

				worldToView_out = worldToView_iter;
				return;
			}
			E_sum_first = E_sum;
		}

		if (correspondences.empty())
		{
			return;
		}

		// Solve
		Eigen::Matrix<double, 6, 1> result;

		if (GUIScene::s_computeICPCPU_Summed)
		{
			Eigen::Matrix<double, 6, 1> b_ = Eigen::Matrix<double, 6, 1>::Zero();
			Eigen::Matrix<double, 6, 6> A_ = Eigen::Matrix<double, 6, 6>::Zero();

			Eigen::Matrix<double, 6, 1> A__ = Eigen::Matrix<double, 6, 1>::Zero();

			for (size_t i = 0; i < correspondences.size(); ++i)
			{
				const auto&     correspondence = correspondences[i];
				Eigen::Vector3d s_i            = GLM2E(std::get<0>(correspondence)).cast<double>();
				Eigen::Vector3d d_i            = GLM2E(std::get<1>(correspondence)).cast<double>();
				Eigen::Vector3d n_i            = GLM2E(std::get<2>(correspondence)).cast<double>();

				Eigen::Matrix<double, 6, 1> A_i;
				A_i << s_i.cross(n_i), n_i;
				A_ += A_i * A_i.transpose();

				b_ += A_i * (n_i.dot(d_i) - n_i.dot(s_i));

				//
				A__ += A_i;
			}

			Eigen::Matrix<double, 6, 6> A___ = A__ * A__.transpose();

			result = A_.bdcSvd(Eigen::ComputeFullU | Eigen::ComputeFullV).solve(b_);
		}
		else
		{
			const size_t    N = correspondences.size();
			Eigen::MatrixXd A(N, 6);
			Eigen::MatrixXd b(N, 1);

			for (size_t i = 0; i < correspondences.size(); ++i)
			{
				const auto&     correspondence = correspondences[i];
				Eigen::Vector3d s_i            = GLM2E(std::get<0>(correspondence)).cast<double>();
				Eigen::Vector3d d_i            = GLM2E(std::get<1>(correspondence)).cast<double>();
				Eigen::Vector3d n_i            = GLM2E(std::get<2>(correspondence)).cast<double>();

				Eigen::Matrix<double, 6, 1> A_i;
				A_i << s_i.cross(n_i), n_i;
				A.row(i) = A_i;

				b(i) = n_i.dot(d_i) - n_i.dot(s_i);
			}

			result = A.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(b);
		}

		if (!isnan(result[0]))
		{
			int test = 1;

			float b  = result[0];
			float g  = result[1];
			float a  = result[2];
			float tx = result[3];
			float ty = result[4];
			float tz = result[5];

			// std::cout << result << std::endl;

			// Update the transformation matrix
			// Eigen::Matrix4f T_inc{{1, a, -g, tx}, {-a, 1, b, ty}, {g, -b, 1, tz}, {0, 0, 0, 1}};
			// viewToWorld_iter = E2GLM(T_inc) * viewToWorld_iter;

			Eigen::Matrix<double, 4, 4> T_inc{{1, a, -g, tx}, {-a, 1, b, ty}, {g, -b, 1, tz}, {0, 0, 0, 1}};

			double alpha = result[0];
			double beta  = result[1];
			double gamma = result[2];

			/* Eigen::Matrix4d transformation{{1, alpha * beta - gamma, alpha * gamma + beta, result[3]},
			                               {gamma, alpha * beta * gamma + 1, beta * gamma - alpha, result[4]},
			                               {-beta, alpha, 1, result[5]},
			                               {0, 0, 0, 1}};*/

			Eigen::Matrix4d transformation{{1, -gamma, beta, result[3]},
			                               {gamma, 1, -alpha, result[4]},
			                               {-beta, alpha, 1, result[5]},
			                               {0, 0, 0, 1}};

			/*
			Eigen::Vector3d   translation = result.tail(3);
			Eigen::Matrix3d rotation = Eigen::AngleAxisd(alpha, Eigen::Vector3d::UnitX()).toRotationMatrix() *
			                           Eigen::AngleAxisd(beta, Eigen::Vector3d::UnitY()).toRotationMatrix() *
			                           Eigen::AngleAxisd(gamma, Eigen::Vector3d::UnitZ()).toRotationMatrix();

			Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();

			pose.block(0, 0, 3, 3) = rotation;
			pose.block(0, 3, 3, 1) = translation;
			*/
			/*
			transformation << 1, alpha * beta - gamma, alpha * gamma + beta, result[3], gamma, alpha * beta * gamma + 1,
			    beta * gamma - alpha, result[4], -beta, alpha, 1, result[5], 0, 0, 0, 1;
			    */
			Eigen::Matrix<double, 4, 4> T_z = GLM2E(viewToWorld_iter);

			T_z = transformation * T_z;

			// Increment
			viewToWorld_iter = E2GLM(T_z);
			worldToView_iter = glm::inverse(viewToWorld_iter);
		}
	}
	worldToView_out = worldToView_iter;
	return;
}

void IterativeClostestPointCPU::getCorrespondences(
    const std::vector<glm::vec3>& oldVertices, const std::vector<glm::vec3>& oldNormals,
    const std::vector<glm::vec3>& newVertices, const std::vector<glm::vec3>& newNormals, glm::mat4x4& viewToWorld_prev,
    glm::mat3x3& viewToWorldRot_prev, glm::highp_dmat4& viewToWorld_iter, glm::mat3x3& viewToWorldRot_iter,
    glm::highp_dmat4& worldToView_iter, const glm::mat4x4& projection, const int& WIDTH, const int& W2, const int& H2,
    const int& SIZE, int& fail_z, double& E_sum, int& found,
    std::vector<std::tuple<glm::vec3, glm::vec3, glm::vec3>>& correspondences)
{
	glm::mat4x4 viewProjection_iter = (projection * glm::mat4x4(worldToView_iter));

	/// ..........................
	for (int i = 0; i < SIZE; i++)
	{
		// TEST MANIPULATION
		// TODO Need to work with camera space coordinates
		glm::vec3 newVertexWorld = viewToWorld_iter * glm::vec4(newVertices[i], 1);

		// TODO FIX
		// Check if there was a valid depth at that vertex
		if (newVertexWorld.z == 0)
		{
			fail_z++;
			continue;
		}

		// NOTE: transform from world to camera -> project
		glm::vec4 clipSpacePos = viewProjection_iter * glm::vec4(newVertexWorld, 1);

		if (clipSpacePos.w == 0)
		{
			m_failPixel++;
			continue;
		}

		glm::vec3 ndc = glm::vec3(clipSpacePos) / clipSpacePos.w;
		if (ndc.x < -1 || ndc.x > 1 || ndc.y < -1 || ndc.y > 1)
		{
			// m_failPixel++;
			continue;
		}

		// OPENGL
		ndc.y      = -ndc.y;
		int x_proj = round((ndc.x + 1) * W2);
		int y_proj = round((ndc.y + 1) * H2);

		int index = y_proj * WIDTH + x_proj;
		if (index < 0 || index >= SIZE)
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
			fail_z++;
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

		// TODO CHECK EPSILONS
		if (glm::dot(referenceNormalWorld, newNormalWorld) < m_epsilonNormal)
		{
			m_failNormal++;
			continue;
		}

		float E = abs(dot(distance, referenceNormalWorld));
		E_sum += E * E;
		found++;

		correspondences.emplace_back(newVertexWorld, referenceVertexWorld, referenceNormalWorld);
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
