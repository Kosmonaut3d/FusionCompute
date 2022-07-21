#include "IterativeClosestPointCPU.h"
#include "../helper/eigen2glm.h"
#include "scenes/GUIScene.h"

//----------------------------------------------------------------------------------------------------------
IterativeClostestPointCPU::IterativeClostestPointCPU()
    : m_epsilonDistance(0.1f)
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

	GUIScene::s_ICP_correspondenceMeasureTime  = 0;
	GUIScene::s_ICP_GPU_reductionMeasureTime   = 0;
	GUIScene::s_ICP_CPU_solveSystemMeasureTime = 0;

	int MAX_IT = GUIScene::s_ICP_iterations;
	for (int it = 0; it < MAX_IT; it++)
	{
		std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

		m_failPixel    = 0;
		m_failDistance = 0;
		m_failNormal   = 0;
		int fail_z     = 0;

		glm::mat3x3 viewToWorldRot_iter = glm::mat3x3(viewToWorld_iter);

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
				// return;
			}
			E_sum_first = E_sum;
		}

		GUIScene::s_ICP_correspondenceMeasureTime +=
		    std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - begin).count() *
		    1000.0;

		if (correspondences.empty())
		{
			return;
		}

		GUIScene::s_ICP_GPU_correspondenceCount = correspondences.size();

		// Solve
		Eigen::Matrix<double, 6, 1> result;

		if (GUIScene::s_ICP_CPU_sum)
		{
			begin = std::chrono::steady_clock::now();

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

			GUIScene::s_ICP_GPU_reductionMeasureTime +=
			    std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - begin)
			        .count() *
			    1000.0;

			begin = std::chrono::steady_clock::now();

			result = A_.bdcSvd(Eigen::ComputeFullU | Eigen::ComputeFullV).solve(b_);

			GUIScene::s_ICP_CPU_solveSystemMeasureTime +=
			    std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - begin)
			        .count() *
			    1000.0;
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

			Eigen::Matrix<double, 4, 4> T_inc{{1, a, -g, tx}, {-a, 1, b, ty}, {g, -b, 1, tz}, {0, 0, 0, 1}};

			double alpha = result[0];
			double beta  = result[1];
			double gamma = result[2];

			Eigen::Matrix4d transformation{{1, -gamma, beta, result[3]},
			                               {gamma, 1, -alpha, result[4]},
			                               {-beta, alpha, 1, result[5]},
			                               {0, 0, 0, 1}};

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
	glm::mat4x4 viewProjection_original = (projection * glm::inverse(viewToWorld_prev));

	/// ..........................
	for (int i = 0; i < SIZE; i++)
	{
		glm::vec3 newVertexWorld = viewToWorld_iter * glm::vec4(newVertices[i], 1);

		// Check if there was a valid depth at that vertex
		if (newVertexWorld.z == 0)
		{
			fail_z++;
			continue;
		}

		// NOTE: transform from world to camera -> project
		glm::vec4 clipSpacePos = viewProjection_original * glm::vec4(newVertexWorld, 1);

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
